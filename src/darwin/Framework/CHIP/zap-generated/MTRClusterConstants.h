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
    MTRClusterIdentifyID API_DEPRECATED(
        "Please use MTRClusterIDTypeIdentifyID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterGroupsID API_DEPRECATED(
        "Please use MTRClusterIDTypeGroupsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterScenesID API_DEPRECATED(
        "Please use MTRClusterIDTypeScenesID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterOnOffID API_DEPRECATED(
        "Please use MTRClusterIDTypeOnOffID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterOnOffSwitchConfigurationID API_DEPRECATED("Please use MTRClusterIDTypeOnOffSwitchConfigurationID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterLevelControlID API_DEPRECATED(
        "Please use MTRClusterIDTypeLevelControlID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterBinaryInputBasicID API_DEPRECATED(
        "Please use MTRClusterIDTypeBinaryInputBasicID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000F,
    MTRClusterPulseWidthModulationID API_DEPRECATED("Please use MTRClusterIDTypePulseWidthModulationID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001C,
    MTRClusterDescriptorID API_DEPRECATED(
        "Please use MTRClusterIDTypeDescriptorID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001D,
    MTRClusterBindingID API_DEPRECATED(
        "Please use MTRClusterIDTypeBindingID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001E,
    MTRClusterAccessControlID API_DEPRECATED(
        "Please use MTRClusterIDTypeAccessControlID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001F,
    MTRClusterActionsID API_DEPRECATED(
        "Please use MTRClusterIDTypeActionsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000025,
    MTRClusterBasicID API_DEPRECATED(
        "Please use MTRClusterIDTypeBasicInformationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000028,
    MTRClusterOtaSoftwareUpdateProviderID API_DEPRECATED("Please use MTRClusterIDTypeOTASoftwareUpdateProviderID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000029,
    MTRClusterOtaSoftwareUpdateRequestorID API_DEPRECATED("Please use MTRClusterIDTypeOTASoftwareUpdateRequestorID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002A,
    MTRClusterLocalizationConfigurationID API_DEPRECATED("Please use MTRClusterIDTypeLocalizationConfigurationID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002B,
    MTRClusterTimeFormatLocalizationID API_DEPRECATED("Please use MTRClusterIDTypeTimeFormatLocalizationID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002C,
    MTRClusterUnitLocalizationID API_DEPRECATED(
        "Please use MTRClusterIDTypeUnitLocalizationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002D,
    MTRClusterPowerSourceConfigurationID API_DEPRECATED("Please use MTRClusterIDTypePowerSourceConfigurationID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002E,
    MTRClusterPowerSourceID API_DEPRECATED(
        "Please use MTRClusterIDTypePowerSourceID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002F,
    MTRClusterGeneralCommissioningID API_DEPRECATED("Please use MTRClusterIDTypeGeneralCommissioningID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000030,
    MTRClusterNetworkCommissioningID API_DEPRECATED("Please use MTRClusterIDTypeNetworkCommissioningID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000031,
    MTRClusterDiagnosticLogsID API_DEPRECATED(
        "Please use MTRClusterIDTypeDiagnosticLogsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000032,
    MTRClusterGeneralDiagnosticsID API_DEPRECATED(
        "Please use MTRClusterIDTypeGeneralDiagnosticsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000033,
    MTRClusterSoftwareDiagnosticsID API_DEPRECATED(
        "Please use MTRClusterIDTypeSoftwareDiagnosticsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000034,
    MTRClusterThreadNetworkDiagnosticsID API_DEPRECATED("Please use MTRClusterIDTypeThreadNetworkDiagnosticsID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000035,
    MTRClusterWiFiNetworkDiagnosticsID API_DEPRECATED("Please use MTRClusterIDTypeWiFiNetworkDiagnosticsID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000036,
    MTRClusterEthernetNetworkDiagnosticsID API_DEPRECATED("Please use MTRClusterIDTypeEthernetNetworkDiagnosticsID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000037,
    MTRClusterTimeSynchronizationID API_DEPRECATED(
        "Please use MTRClusterIDTypeTimeSynchronizationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000038,
    MTRClusterBridgedDeviceBasicID API_DEPRECATED("Please use MTRClusterIDTypeBridgedDeviceBasicInformationID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000039,
    MTRClusterSwitchID API_DEPRECATED(
        "Please use MTRClusterIDTypeSwitchID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000003B,
    MTRClusterAdministratorCommissioningID API_DEPRECATED("Please use MTRClusterIDTypeAdministratorCommissioningID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000003C,
    MTRClusterOperationalCredentialsID API_DEPRECATED("Please use MTRClusterIDTypeOperationalCredentialsID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000003E,
    MTRClusterGroupKeyManagementID API_DEPRECATED(
        "Please use MTRClusterIDTypeGroupKeyManagementID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000003F,
    MTRClusterFixedLabelID API_DEPRECATED(
        "Please use MTRClusterIDTypeFixedLabelID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000040,
    MTRClusterUserLabelID API_DEPRECATED(
        "Please use MTRClusterIDTypeUserLabelID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000041,
    MTRClusterProxyConfigurationID API_DEPRECATED(
        "Please use MTRClusterIDTypeProxyConfigurationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000042,
    MTRClusterProxyDiscoveryID API_DEPRECATED(
        "Please use MTRClusterIDTypeProxyDiscoveryID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000043,
    MTRClusterProxyValidID API_DEPRECATED(
        "Please use MTRClusterIDTypeProxyValidID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000044,
    MTRClusterBooleanStateID API_DEPRECATED(
        "Please use MTRClusterIDTypeBooleanStateID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000045,
    MTRClusterModeSelectID API_DEPRECATED(
        "Please use MTRClusterIDTypeModeSelectID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000050,
    MTRClusterDoorLockID API_DEPRECATED(
        "Please use MTRClusterIDTypeDoorLockID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000101,
    MTRClusterWindowCoveringID API_DEPRECATED(
        "Please use MTRClusterIDTypeWindowCoveringID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000102,
    MTRClusterBarrierControlID API_DEPRECATED(
        "Please use MTRClusterIDTypeBarrierControlID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000103,
    MTRClusterPumpConfigurationAndControlID API_DEPRECATED("Please use MTRClusterIDTypePumpConfigurationAndControlID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000200,
    MTRClusterThermostatID API_DEPRECATED(
        "Please use MTRClusterIDTypeThermostatID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000201,
    MTRClusterFanControlID API_DEPRECATED(
        "Please use MTRClusterIDTypeFanControlID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000202,
    MTRClusterThermostatUserInterfaceConfigurationID API_DEPRECATED(
        "Please use MTRClusterIDTypeThermostatUserInterfaceConfigurationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4),
        tvos(16.1, 16.4))
    = 0x00000204,
    MTRClusterColorControlID API_DEPRECATED(
        "Please use MTRClusterIDTypeColorControlID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000300,
    MTRClusterBallastConfigurationID API_DEPRECATED("Please use MTRClusterIDTypeBallastConfigurationID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000301,
    MTRClusterIlluminanceMeasurementID API_DEPRECATED("Please use MTRClusterIDTypeIlluminanceMeasurementID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000400,
    MTRClusterTemperatureMeasurementID API_DEPRECATED("Please use MTRClusterIDTypeTemperatureMeasurementID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000402,
    MTRClusterPressureMeasurementID API_DEPRECATED(
        "Please use MTRClusterIDTypePressureMeasurementID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000403,
    MTRClusterFlowMeasurementID API_DEPRECATED(
        "Please use MTRClusterIDTypeFlowMeasurementID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000404,
    MTRClusterRelativeHumidityMeasurementID API_DEPRECATED("Please use MTRClusterIDTypeRelativeHumidityMeasurementID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000405,
    MTRClusterOccupancySensingID API_DEPRECATED(
        "Please use MTRClusterIDTypeOccupancySensingID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000406,
    MTRClusterWakeOnLanID API_DEPRECATED(
        "Please use MTRClusterIDTypeWakeOnLANID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000503,
    MTRClusterChannelID API_DEPRECATED(
        "Please use MTRClusterIDTypeChannelID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000504,
    MTRClusterTargetNavigatorID API_DEPRECATED(
        "Please use MTRClusterIDTypeTargetNavigatorID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000505,
    MTRClusterMediaPlaybackID API_DEPRECATED(
        "Please use MTRClusterIDTypeMediaPlaybackID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000506,
    MTRClusterMediaInputID API_DEPRECATED(
        "Please use MTRClusterIDTypeMediaInputID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000507,
    MTRClusterLowPowerID API_DEPRECATED(
        "Please use MTRClusterIDTypeLowPowerID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000508,
    MTRClusterKeypadInputID API_DEPRECATED(
        "Please use MTRClusterIDTypeKeypadInputID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000509,
    MTRClusterContentLauncherID API_DEPRECATED(
        "Please use MTRClusterIDTypeContentLauncherID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000050A,
    MTRClusterAudioOutputID API_DEPRECATED(
        "Please use MTRClusterIDTypeAudioOutputID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000050B,
    MTRClusterApplicationLauncherID API_DEPRECATED(
        "Please use MTRClusterIDTypeApplicationLauncherID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000050C,
    MTRClusterApplicationBasicID API_DEPRECATED(
        "Please use MTRClusterIDTypeApplicationBasicID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000050D,
    MTRClusterAccountLoginID API_DEPRECATED(
        "Please use MTRClusterIDTypeAccountLoginID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000050E,
    MTRClusterElectricalMeasurementID API_DEPRECATED("Please use MTRClusterIDTypeElectricalMeasurementID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000B04,
    MTRClusterTestClusterID API_DEPRECATED(
        "Please use MTRClusterIDTypeUnitTestingID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0xFFF1FC05,
    MTRClusterFaultInjectionID API_DEPRECATED(
        "Please use MTRClusterIDTypeFaultInjectionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0xFFF1FC06,
    MTRClusterIDTypeIdentifyID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRClusterIDTypeGroupsID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRClusterIDTypeScenesID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRClusterIDTypeOnOffID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTRClusterIDTypeOnOffSwitchConfigurationID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000007,
    MTRClusterIDTypeLevelControlID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000008,
    MTRClusterIDTypeBinaryInputBasicID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000F,
    MTRClusterIDTypePulseWidthModulationID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001C,
    MTRClusterIDTypeDescriptorID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001D,
    MTRClusterIDTypeBindingID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001E,
    MTRClusterIDTypeAccessControlID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001F,
    MTRClusterIDTypeActionsID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000025,
    MTRClusterIDTypeBasicInformationID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000028,
    MTRClusterIDTypeOTASoftwareUpdateProviderID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000029,
    MTRClusterIDTypeOTASoftwareUpdateRequestorID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000002A,
    MTRClusterIDTypeLocalizationConfigurationID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000002B,
    MTRClusterIDTypeTimeFormatLocalizationID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000002C,
    MTRClusterIDTypeUnitLocalizationID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000002D,
    MTRClusterIDTypePowerSourceConfigurationID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000002E,
    MTRClusterIDTypePowerSourceID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000002F,
    MTRClusterIDTypeGeneralCommissioningID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000030,
    MTRClusterIDTypeNetworkCommissioningID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000031,
    MTRClusterIDTypeDiagnosticLogsID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000032,
    MTRClusterIDTypeGeneralDiagnosticsID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000033,
    MTRClusterIDTypeSoftwareDiagnosticsID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000034,
    MTRClusterIDTypeThreadNetworkDiagnosticsID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000035,
    MTRClusterIDTypeWiFiNetworkDiagnosticsID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000036,
    MTRClusterIDTypeEthernetNetworkDiagnosticsID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000037,
    MTRClusterIDTypeTimeSynchronizationID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000038,
    MTRClusterIDTypeBridgedDeviceBasicInformationID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000039,
    MTRClusterIDTypeSwitchID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000003B,
    MTRClusterIDTypeAdministratorCommissioningID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000003C,
    MTRClusterIDTypeOperationalCredentialsID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000003E,
    MTRClusterIDTypeGroupKeyManagementID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000003F,
    MTRClusterIDTypeFixedLabelID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000040,
    MTRClusterIDTypeUserLabelID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000041,
    MTRClusterIDTypeProxyConfigurationID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000042,
    MTRClusterIDTypeProxyDiscoveryID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000043,
    MTRClusterIDTypeProxyValidID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000044,
    MTRClusterIDTypeBooleanStateID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000045,
    MTRClusterIDTypeModeSelectID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000050,
    MTRClusterIDTypeDoorLockID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000101,
    MTRClusterIDTypeWindowCoveringID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000102,
    MTRClusterIDTypeBarrierControlID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000103,
    MTRClusterIDTypePumpConfigurationAndControlID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000200,
    MTRClusterIDTypeThermostatID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000201,
    MTRClusterIDTypeFanControlID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000202,
    MTRClusterIDTypeThermostatUserInterfaceConfigurationID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000204,
    MTRClusterIDTypeColorControlID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000300,
    MTRClusterIDTypeBallastConfigurationID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000301,
    MTRClusterIDTypeIlluminanceMeasurementID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000400,
    MTRClusterIDTypeTemperatureMeasurementID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000402,
    MTRClusterIDTypePressureMeasurementID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000403,
    MTRClusterIDTypeFlowMeasurementID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000404,
    MTRClusterIDTypeRelativeHumidityMeasurementID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000405,
    MTRClusterIDTypeOccupancySensingID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000406,
    MTRClusterIDTypeWakeOnLANID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000503,
    MTRClusterIDTypeChannelID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000504,
    MTRClusterIDTypeTargetNavigatorID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000505,
    MTRClusterIDTypeMediaPlaybackID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000506,
    MTRClusterIDTypeMediaInputID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000507,
    MTRClusterIDTypeLowPowerID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000508,
    MTRClusterIDTypeKeypadInputID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000509,
    MTRClusterIDTypeContentLauncherID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000050A,
    MTRClusterIDTypeAudioOutputID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000050B,
    MTRClusterIDTypeApplicationLauncherID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000050C,
    MTRClusterIDTypeApplicationBasicID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000050D,
    MTRClusterIDTypeAccountLoginID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000050E,
    MTRClusterIDTypeElectricalMeasurementID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000B04,
    MTRClusterIDTypeUnitTestingID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0xFFF1FC05,
    MTRClusterIDTypeFaultInjectionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0xFFF1FC06,
};

#pragma mark - Attributes IDs

typedef NS_ENUM(uint32_t, MTRAttributeIDType) {
    // Deprecated global attribute names
    MTRClusterGlobalAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4),
        tvos(16.1, 16.4))
    = 0x0000FFF8,
    MTRClusterGlobalAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4),
        tvos(16.1, 16.4))
    = 0x0000FFF9,
    MTRClusterGlobalAttributeAttributeListID API_DEPRECATED("Please use MTRAttributeIDTypeGlobalAttributeAttributeListID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000FFFB,
    MTRClusterGlobalAttributeFeatureMapID API_DEPRECATED("Please use MTRAttributeIDTypeGlobalAttributeFeatureMapID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000FFFC,
    MTRClusterGlobalAttributeClusterRevisionID API_DEPRECATED("Please use MTRAttributeIDTypeGlobalAttributeClusterRevisionID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000FFFD,

    // Global attributes
    MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000FFF8,
    MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000FFF9,
    MTRAttributeIDTypeGlobalAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000FFFB,
    MTRAttributeIDTypeGlobalAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000FFFC,
    MTRAttributeIDTypeGlobalAttributeClusterRevisionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000FFFD,

    // Cluster Identify deprecated attribute names
    MTRClusterIdentifyAttributeIdentifyTimeID API_DEPRECATED("Please use MTRAttributeIDTypeClusterIdentifyAttributeIdentifyTimeID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterIdentifyAttributeIdentifyTypeID API_DEPRECATED("Please use MTRAttributeIDTypeClusterIdentifyAttributeIdentifyTypeID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterIdentifyAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIdentifyAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterIdentifyAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIdentifyAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterIdentifyAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIdentifyAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterIdentifyAttributeFeatureMapID API_DEPRECATED("Please use MTRAttributeIDTypeClusterIdentifyAttributeFeatureMapID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterIdentifyAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIdentifyAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster Identify attributes
    MTRAttributeIDTypeClusterIdentifyAttributeIdentifyTimeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterIdentifyAttributeIdentifyTypeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterIdentifyAttributeGeneratedCommandListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterIdentifyAttributeAcceptedCommandListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterIdentifyAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterIdentifyAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterIdentifyAttributeClusterRevisionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Groups deprecated attribute names
    MTRClusterGroupsAttributeNameSupportID API_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupsAttributeNameSupportID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterGroupsAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGroupsAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterGroupsAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGroupsAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterGroupsAttributeAttributeListID API_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupsAttributeAttributeListID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterGroupsAttributeFeatureMapID API_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupsAttributeFeatureMapID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterGroupsAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGroupsAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster Groups attributes
    MTRAttributeIDTypeClusterGroupsAttributeNameSupportID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterGroupsAttributeGeneratedCommandListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterGroupsAttributeAcceptedCommandListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterGroupsAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterGroupsAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterGroupsAttributeClusterRevisionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Scenes deprecated attribute names
    MTRClusterScenesAttributeSceneCountID API_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeSceneCountID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterScenesAttributeCurrentSceneID API_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeCurrentSceneID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterScenesAttributeCurrentGroupID API_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeCurrentGroupID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterScenesAttributeSceneValidID API_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeSceneValidID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterScenesAttributeNameSupportID API_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeNameSupportID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterScenesAttributeLastConfiguredByID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterScenesAttributeLastConfiguredByID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterScenesAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterScenesAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterScenesAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterScenesAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterScenesAttributeAttributeListID API_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeAttributeListID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterScenesAttributeFeatureMapID API_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeFeatureMapID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterScenesAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterScenesAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster Scenes attributes
    MTRAttributeIDTypeClusterScenesAttributeSceneCountID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterScenesAttributeCurrentSceneID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterScenesAttributeCurrentGroupID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterScenesAttributeSceneValidID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterScenesAttributeNameSupportID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRAttributeIDTypeClusterScenesAttributeLastConfiguredByID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTRAttributeIDTypeClusterScenesAttributeGeneratedCommandListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterScenesAttributeAcceptedCommandListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterScenesAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterScenesAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterScenesAttributeClusterRevisionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster OnOff deprecated attribute names
    MTRClusterOnOffAttributeOnOffID API_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeOnOffID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterOnOffAttributeGlobalSceneControlID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOnOffAttributeGlobalSceneControlID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004000,
    MTRClusterOnOffAttributeOnTimeID API_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeOnTimeID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004001,
    MTRClusterOnOffAttributeOffWaitTimeID API_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeOffWaitTimeID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004002,
    MTRClusterOnOffAttributeStartUpOnOffID API_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeStartUpOnOffID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004003,
    MTRClusterOnOffAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOnOffAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterOnOffAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOnOffAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterOnOffAttributeAttributeListID API_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeAttributeListID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterOnOffAttributeFeatureMapID API_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeFeatureMapID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterOnOffAttributeClusterRevisionID API_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeClusterRevisionID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster OnOff attributes
    MTRAttributeIDTypeClusterOnOffAttributeOnOffID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterOnOffAttributeGlobalSceneControlID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004000,
    MTRAttributeIDTypeClusterOnOffAttributeOnTimeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004001,
    MTRAttributeIDTypeClusterOnOffAttributeOffWaitTimeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004002,
    MTRAttributeIDTypeClusterOnOffAttributeStartUpOnOffID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004003,
    MTRAttributeIDTypeClusterOnOffAttributeGeneratedCommandListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterOnOffAttributeAcceptedCommandListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterOnOffAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterOnOffAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterOnOffAttributeClusterRevisionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster OnOffSwitchConfiguration deprecated attribute names
    MTRClusterOnOffSwitchConfigurationAttributeSwitchTypeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeSwitchTypeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterOnOffSwitchConfigurationAttributeSwitchActionsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeSwitchActionsID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,
    MTRClusterOnOffSwitchConfigurationAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeGeneratedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterOnOffSwitchConfigurationAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeAcceptedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterOnOffSwitchConfigurationAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterOnOffSwitchConfigurationAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterOnOffSwitchConfigurationAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeClusterRevisionID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster OnOffSwitchConfiguration attributes
    MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeSwitchTypeID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeSwitchActionsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000010,
    MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeFeatureMapID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster LevelControl deprecated attribute names
    MTRClusterLevelControlAttributeCurrentLevelID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeCurrentLevelID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterLevelControlAttributeRemainingTimeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeRemainingTimeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterLevelControlAttributeMinLevelID API_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeMinLevelID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterLevelControlAttributeMaxLevelID API_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeMaxLevelID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterLevelControlAttributeCurrentFrequencyID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeCurrentFrequencyID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterLevelControlAttributeMinFrequencyID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeMinFrequencyID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterLevelControlAttributeMaxFrequencyID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeMaxFrequencyID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterLevelControlAttributeOptionsID API_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeOptionsID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000F,
    MTRClusterLevelControlAttributeOnOffTransitionTimeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeOnOffTransitionTimeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,
    MTRClusterLevelControlAttributeOnLevelID API_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeOnLevelID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterLevelControlAttributeOnTransitionTimeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeOnTransitionTimeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterLevelControlAttributeOffTransitionTimeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeOffTransitionTimeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000013,
    MTRClusterLevelControlAttributeDefaultMoveRateID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeDefaultMoveRateID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000014,
    MTRClusterLevelControlAttributeStartUpCurrentLevelID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeStartUpCurrentLevelID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004000,
    MTRClusterLevelControlAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterLevelControlAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterLevelControlAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterLevelControlAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterLevelControlAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster LevelControl attributes
    MTRAttributeIDTypeClusterLevelControlAttributeCurrentLevelID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterLevelControlAttributeRemainingTimeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterLevelControlAttributeMinLevelID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterLevelControlAttributeMaxLevelID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterLevelControlAttributeCurrentFrequencyID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRAttributeIDTypeClusterLevelControlAttributeMinFrequencyID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTRAttributeIDTypeClusterLevelControlAttributeMaxFrequencyID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,
    MTRAttributeIDTypeClusterLevelControlAttributeOptionsID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000F,
    MTRAttributeIDTypeClusterLevelControlAttributeOnOffTransitionTimeID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000010,
    MTRAttributeIDTypeClusterLevelControlAttributeOnLevelID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000011,
    MTRAttributeIDTypeClusterLevelControlAttributeOnTransitionTimeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000012,
    MTRAttributeIDTypeClusterLevelControlAttributeOffTransitionTimeID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000013,
    MTRAttributeIDTypeClusterLevelControlAttributeDefaultMoveRateID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000014,
    MTRAttributeIDTypeClusterLevelControlAttributeStartUpCurrentLevelID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004000,
    MTRAttributeIDTypeClusterLevelControlAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterLevelControlAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterLevelControlAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterLevelControlAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterLevelControlAttributeClusterRevisionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster BinaryInputBasic deprecated attribute names
    MTRClusterBinaryInputBasicAttributeActiveTextID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeActiveTextID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterBinaryInputBasicAttributeDescriptionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeDescriptionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001C,
    MTRClusterBinaryInputBasicAttributeInactiveTextID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeInactiveTextID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002E,
    MTRClusterBinaryInputBasicAttributeOutOfServiceID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeOutOfServiceID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000051,
    MTRClusterBinaryInputBasicAttributePolarityID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributePolarityID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000054,
    MTRClusterBinaryInputBasicAttributePresentValueID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributePresentValueID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000055,
    MTRClusterBinaryInputBasicAttributeReliabilityID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeReliabilityID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000067,
    MTRClusterBinaryInputBasicAttributeStatusFlagsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeStatusFlagsID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000006F,
    MTRClusterBinaryInputBasicAttributeApplicationTypeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeApplicationTypeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000100,
    MTRClusterBinaryInputBasicAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterBinaryInputBasicAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterBinaryInputBasicAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterBinaryInputBasicAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterBinaryInputBasicAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster BinaryInputBasic attributes
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeActiveTextID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeDescriptionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000001C,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeInactiveTextID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000002E,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeOutOfServiceID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000051,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributePolarityID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000054,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributePresentValueID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000055,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeReliabilityID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000067,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeStatusFlagsID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000006F,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeApplicationTypeID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000100,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster PulseWidthModulation deprecated attribute names
    MTRClusterPulseWidthModulationAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPulseWidthModulationAttributeGeneratedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterPulseWidthModulationAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPulseWidthModulationAttributeAcceptedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterPulseWidthModulationAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPulseWidthModulationAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterPulseWidthModulationAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPulseWidthModulationAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterPulseWidthModulationAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPulseWidthModulationAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster PulseWidthModulation attributes
    MTRAttributeIDTypeClusterPulseWidthModulationAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterPulseWidthModulationAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterPulseWidthModulationAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterPulseWidthModulationAttributeFeatureMapID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterPulseWidthModulationAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Descriptor deprecated attribute names
    MTRClusterDescriptorAttributeDeviceTypeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDescriptorAttributeDeviceTypeListID", ios(16.2, 16.4), macos(13.1, 13.3),
        watchos(9.2, 9.4), tvos(16.2, 16.4))
    = 0x00000000,
    MTRClusterDescriptorAttributeDeviceListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDescriptorAttributeDeviceTypeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterDescriptorAttributeServerListID API_DEPRECATED("Please use MTRAttributeIDTypeClusterDescriptorAttributeServerListID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterDescriptorAttributeClientListID API_DEPRECATED("Please use MTRAttributeIDTypeClusterDescriptorAttributeClientListID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterDescriptorAttributePartsListID API_DEPRECATED("Please use MTRAttributeIDTypeClusterDescriptorAttributePartsListID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterDescriptorAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDescriptorAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterDescriptorAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDescriptorAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterDescriptorAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDescriptorAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterDescriptorAttributeFeatureMapID API_DEPRECATED("Please use MTRAttributeIDTypeClusterDescriptorAttributeFeatureMapID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterDescriptorAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDescriptorAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster Descriptor attributes
    MTRAttributeIDTypeClusterDescriptorAttributeDeviceTypeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterDescriptorAttributeServerListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterDescriptorAttributeClientListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterDescriptorAttributePartsListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterDescriptorAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterDescriptorAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterDescriptorAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterDescriptorAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterDescriptorAttributeClusterRevisionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Binding deprecated attribute names
    MTRClusterBindingAttributeBindingID API_DEPRECATED("Please use MTRAttributeIDTypeClusterBindingAttributeBindingID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterBindingAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBindingAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterBindingAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBindingAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterBindingAttributeAttributeListID API_DEPRECATED("Please use MTRAttributeIDTypeClusterBindingAttributeAttributeListID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterBindingAttributeFeatureMapID API_DEPRECATED("Please use MTRAttributeIDTypeClusterBindingAttributeFeatureMapID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterBindingAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBindingAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster Binding attributes
    MTRAttributeIDTypeClusterBindingAttributeBindingID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterBindingAttributeGeneratedCommandListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBindingAttributeAcceptedCommandListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBindingAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBindingAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBindingAttributeClusterRevisionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster AccessControl deprecated attribute names
    MTRClusterAccessControlAttributeAclID API_DEPRECATED("Please use MTRAttributeIDTypeClusterAccessControlAttributeACLID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterAccessControlAttributeExtensionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAccessControlAttributeExtensionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterAccessControlAttributeSubjectsPerAccessControlEntryID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAccessControlAttributeSubjectsPerAccessControlEntryID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterAccessControlAttributeTargetsPerAccessControlEntryID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAccessControlAttributeTargetsPerAccessControlEntryID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterAccessControlAttributeAccessControlEntriesPerFabricID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAccessControlAttributeAccessControlEntriesPerFabricID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterAccessControlAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAccessControlAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterAccessControlAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAccessControlAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterAccessControlAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAccessControlAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterAccessControlAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAccessControlAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterAccessControlAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAccessControlAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster AccessControl attributes
    MTRAttributeIDTypeClusterAccessControlAttributeACLID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterAccessControlAttributeExtensionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterAccessControlAttributeSubjectsPerAccessControlEntryID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterAccessControlAttributeTargetsPerAccessControlEntryID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterAccessControlAttributeAccessControlEntriesPerFabricID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRAttributeIDTypeClusterAccessControlAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterAccessControlAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterAccessControlAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterAccessControlAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterAccessControlAttributeClusterRevisionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Actions deprecated attribute names
    MTRClusterActionsAttributeActionListID API_DEPRECATED("Please use MTRAttributeIDTypeClusterActionsAttributeActionListID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterActionsAttributeEndpointListsID API_DEPRECATED("Please use MTRAttributeIDTypeClusterActionsAttributeEndpointListsID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterActionsAttributeSetupURLID API_DEPRECATED("Please use MTRAttributeIDTypeClusterActionsAttributeSetupURLID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterActionsAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterActionsAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterActionsAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterActionsAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterActionsAttributeAttributeListID API_DEPRECATED("Please use MTRAttributeIDTypeClusterActionsAttributeAttributeListID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterActionsAttributeFeatureMapID API_DEPRECATED("Please use MTRAttributeIDTypeClusterActionsAttributeFeatureMapID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterActionsAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterActionsAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster Actions attributes
    MTRAttributeIDTypeClusterActionsAttributeActionListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterActionsAttributeEndpointListsID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterActionsAttributeSetupURLID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterActionsAttributeGeneratedCommandListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterActionsAttributeAcceptedCommandListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterActionsAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterActionsAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterActionsAttributeClusterRevisionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Basic deprecated attribute names
    MTRClusterBasicAttributeDataModelRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBasicInformationAttributeDataModelRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterBasicAttributeVendorNameID API_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeVendorNameID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterBasicAttributeVendorIDID API_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeVendorIDID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterBasicAttributeProductNameID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBasicInformationAttributeProductNameID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterBasicAttributeProductIDID API_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeProductIDID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterBasicAttributeNodeLabelID API_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeNodeLabelID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterBasicAttributeLocationID API_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeLocationID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterBasicAttributeHardwareVersionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBasicInformationAttributeHardwareVersionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterBasicAttributeHardwareVersionStringID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBasicInformationAttributeHardwareVersionStringID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterBasicAttributeSoftwareVersionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBasicInformationAttributeSoftwareVersionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterBasicAttributeSoftwareVersionStringID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBasicInformationAttributeSoftwareVersionStringID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterBasicAttributeManufacturingDateID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBasicInformationAttributeManufacturingDateID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000B,
    MTRClusterBasicAttributePartNumberID API_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributePartNumberID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000C,
    MTRClusterBasicAttributeProductURLID API_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeProductURLID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000D,
    MTRClusterBasicAttributeProductLabelID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBasicInformationAttributeProductLabelID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000E,
    MTRClusterBasicAttributeSerialNumberID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBasicInformationAttributeSerialNumberID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000F,
    MTRClusterBasicAttributeLocalConfigDisabledID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBasicInformationAttributeLocalConfigDisabledID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,
    MTRClusterBasicAttributeReachableID API_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeReachableID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterBasicAttributeUniqueIDID API_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeUniqueIDID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterBasicAttributeCapabilityMinimaID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBasicInformationAttributeCapabilityMinimaID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000013,
    MTRClusterBasicAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBasicInformationAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterBasicAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBasicInformationAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterBasicAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBasicInformationAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterBasicAttributeFeatureMapID API_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeFeatureMapID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterBasicAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBasicInformationAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster BasicInformation attributes
    MTRAttributeIDTypeClusterBasicInformationAttributeDataModelRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterBasicInformationAttributeVendorNameID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterBasicInformationAttributeVendorIDID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterBasicInformationAttributeProductNameID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterBasicInformationAttributeProductIDID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRAttributeIDTypeClusterBasicInformationAttributeNodeLabelID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTRAttributeIDTypeClusterBasicInformationAttributeLocationID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,
    MTRAttributeIDTypeClusterBasicInformationAttributeHardwareVersionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000007,
    MTRAttributeIDTypeClusterBasicInformationAttributeHardwareVersionStringID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000008,
    MTRAttributeIDTypeClusterBasicInformationAttributeSoftwareVersionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000009,
    MTRAttributeIDTypeClusterBasicInformationAttributeSoftwareVersionStringID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000A,
    MTRAttributeIDTypeClusterBasicInformationAttributeManufacturingDateID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000B,
    MTRAttributeIDTypeClusterBasicInformationAttributePartNumberID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000C,
    MTRAttributeIDTypeClusterBasicInformationAttributeProductURLID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000D,
    MTRAttributeIDTypeClusterBasicInformationAttributeProductLabelID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000E,
    MTRAttributeIDTypeClusterBasicInformationAttributeSerialNumberID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000F,
    MTRAttributeIDTypeClusterBasicInformationAttributeLocalConfigDisabledID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000010,
    MTRAttributeIDTypeClusterBasicInformationAttributeReachableID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000011,
    MTRAttributeIDTypeClusterBasicInformationAttributeUniqueIDID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000012,
    MTRAttributeIDTypeClusterBasicInformationAttributeCapabilityMinimaID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000013,
    MTRAttributeIDTypeClusterBasicInformationAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBasicInformationAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBasicInformationAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBasicInformationAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBasicInformationAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster OtaSoftwareUpdateProvider deprecated attribute names
    MTRClusterOtaSoftwareUpdateProviderAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeGeneratedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterOtaSoftwareUpdateProviderAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeAcceptedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterOtaSoftwareUpdateProviderAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterOtaSoftwareUpdateProviderAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterOtaSoftwareUpdateProviderAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeClusterRevisionID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster OTASoftwareUpdateProvider attributes
    MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeFeatureMapID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster OtaSoftwareUpdateRequestor deprecated attribute names
    MTRClusterOtaSoftwareUpdateRequestorAttributeDefaultOtaProvidersID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeDefaultOTAProvidersID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterOtaSoftwareUpdateRequestorAttributeUpdatePossibleID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeUpdatePossibleID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterOtaSoftwareUpdateRequestorAttributeUpdateStateID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeUpdateStateID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterOtaSoftwareUpdateRequestorAttributeUpdateStateProgressID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeUpdateStateProgressID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterOtaSoftwareUpdateRequestorAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeGeneratedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterOtaSoftwareUpdateRequestorAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeAcceptedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterOtaSoftwareUpdateRequestorAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeAttributeListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterOtaSoftwareUpdateRequestorAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterOtaSoftwareUpdateRequestorAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeClusterRevisionID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster OTASoftwareUpdateRequestor attributes
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeDefaultOTAProvidersID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeUpdatePossibleID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeUpdateStateID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeUpdateStateProgressID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeFeatureMapID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster LocalizationConfiguration deprecated attribute names
    MTRClusterLocalizationConfigurationAttributeActiveLocaleID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLocalizationConfigurationAttributeActiveLocaleID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterLocalizationConfigurationAttributeSupportedLocalesID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLocalizationConfigurationAttributeSupportedLocalesID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterLocalizationConfigurationAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLocalizationConfigurationAttributeGeneratedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterLocalizationConfigurationAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLocalizationConfigurationAttributeAcceptedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterLocalizationConfigurationAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLocalizationConfigurationAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterLocalizationConfigurationAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLocalizationConfigurationAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterLocalizationConfigurationAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLocalizationConfigurationAttributeClusterRevisionID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster LocalizationConfiguration attributes
    MTRAttributeIDTypeClusterLocalizationConfigurationAttributeActiveLocaleID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterLocalizationConfigurationAttributeSupportedLocalesID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterLocalizationConfigurationAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterLocalizationConfigurationAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterLocalizationConfigurationAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterLocalizationConfigurationAttributeFeatureMapID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterLocalizationConfigurationAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster TimeFormatLocalization deprecated attribute names
    MTRClusterTimeFormatLocalizationAttributeHourFormatID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeHourFormatID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterTimeFormatLocalizationAttributeActiveCalendarTypeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeActiveCalendarTypeID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterTimeFormatLocalizationAttributeSupportedCalendarTypesID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeSupportedCalendarTypesID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterTimeFormatLocalizationAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeGeneratedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterTimeFormatLocalizationAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeAcceptedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterTimeFormatLocalizationAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterTimeFormatLocalizationAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterTimeFormatLocalizationAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster TimeFormatLocalization attributes
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeHourFormatID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeActiveCalendarTypeID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeSupportedCalendarTypesID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeFeatureMapID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster UnitLocalization deprecated attribute names
    MTRClusterUnitLocalizationAttributeTemperatureUnitID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitLocalizationAttributeTemperatureUnitID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterUnitLocalizationAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitLocalizationAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterUnitLocalizationAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitLocalizationAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterUnitLocalizationAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitLocalizationAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterUnitLocalizationAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitLocalizationAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterUnitLocalizationAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitLocalizationAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster UnitLocalization attributes
    MTRAttributeIDTypeClusterUnitLocalizationAttributeTemperatureUnitID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterUnitLocalizationAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterUnitLocalizationAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterUnitLocalizationAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterUnitLocalizationAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterUnitLocalizationAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster PowerSourceConfiguration deprecated attribute names
    MTRClusterPowerSourceConfigurationAttributeSourcesID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeSourcesID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterPowerSourceConfigurationAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeGeneratedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterPowerSourceConfigurationAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeAcceptedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterPowerSourceConfigurationAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterPowerSourceConfigurationAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterPowerSourceConfigurationAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeClusterRevisionID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster PowerSourceConfiguration attributes
    MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeSourcesID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeFeatureMapID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster PowerSource deprecated attribute names
    MTRClusterPowerSourceAttributeStatusID API_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeStatusID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterPowerSourceAttributeOrderID API_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeOrderID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterPowerSourceAttributeDescriptionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeDescriptionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterPowerSourceAttributeWiredAssessedInputVoltageID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeWiredAssessedInputVoltageID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterPowerSourceAttributeWiredAssessedInputFrequencyID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeWiredAssessedInputFrequencyID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterPowerSourceAttributeWiredCurrentTypeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeWiredCurrentTypeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterPowerSourceAttributeWiredAssessedCurrentID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeWiredAssessedCurrentID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterPowerSourceAttributeWiredNominalVoltageID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeWiredNominalVoltageID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterPowerSourceAttributeWiredMaximumCurrentID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeWiredMaximumCurrentID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterPowerSourceAttributeWiredPresentID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeWiredPresentID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterPowerSourceAttributeActiveWiredFaultsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeActiveWiredFaultsID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterPowerSourceAttributeBatVoltageID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatVoltageID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000B,
    MTRClusterPowerSourceAttributeBatPercentRemainingID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatPercentRemainingID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000C,
    MTRClusterPowerSourceAttributeBatTimeRemainingID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatTimeRemainingID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000D,
    MTRClusterPowerSourceAttributeBatChargeLevelID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatChargeLevelID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000E,
    MTRClusterPowerSourceAttributeBatReplacementNeededID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatReplacementNeededID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000F,
    MTRClusterPowerSourceAttributeBatReplaceabilityID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatReplaceabilityID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,
    MTRClusterPowerSourceAttributeBatPresentID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatPresentID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterPowerSourceAttributeActiveBatFaultsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeActiveBatFaultsID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterPowerSourceAttributeBatReplacementDescriptionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatReplacementDescriptionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000013,
    MTRClusterPowerSourceAttributeBatCommonDesignationID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatCommonDesignationID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000014,
    MTRClusterPowerSourceAttributeBatANSIDesignationID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatANSIDesignationID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000015,
    MTRClusterPowerSourceAttributeBatIECDesignationID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatIECDesignationID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000016,
    MTRClusterPowerSourceAttributeBatApprovedChemistryID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatApprovedChemistryID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000017,
    MTRClusterPowerSourceAttributeBatCapacityID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatCapacityID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000018,
    MTRClusterPowerSourceAttributeBatQuantityID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatQuantityID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000019,
    MTRClusterPowerSourceAttributeBatChargeStateID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatChargeStateID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001A,
    MTRClusterPowerSourceAttributeBatTimeToFullChargeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatTimeToFullChargeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001B,
    MTRClusterPowerSourceAttributeBatFunctionalWhileChargingID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatFunctionalWhileChargingID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001C,
    MTRClusterPowerSourceAttributeBatChargingCurrentID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatChargingCurrentID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001D,
    MTRClusterPowerSourceAttributeActiveBatChargeFaultsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeActiveBatChargeFaultsID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001E,
    MTRClusterPowerSourceAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterPowerSourceAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterPowerSourceAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterPowerSourceAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterPowerSourceAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster PowerSource attributes
    MTRAttributeIDTypeClusterPowerSourceAttributeStatusID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterPowerSourceAttributeOrderID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterPowerSourceAttributeDescriptionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterPowerSourceAttributeWiredAssessedInputVoltageID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterPowerSourceAttributeWiredAssessedInputFrequencyID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRAttributeIDTypeClusterPowerSourceAttributeWiredCurrentTypeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTRAttributeIDTypeClusterPowerSourceAttributeWiredAssessedCurrentID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,
    MTRAttributeIDTypeClusterPowerSourceAttributeWiredNominalVoltageID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000007,
    MTRAttributeIDTypeClusterPowerSourceAttributeWiredMaximumCurrentID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000008,
    MTRAttributeIDTypeClusterPowerSourceAttributeWiredPresentID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000009,
    MTRAttributeIDTypeClusterPowerSourceAttributeActiveWiredFaultsID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000A,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatVoltageID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000B,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatPercentRemainingID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000C,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatTimeRemainingID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000D,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatChargeLevelID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000E,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatReplacementNeededID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000F,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatReplaceabilityID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000010,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatPresentID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000011,
    MTRAttributeIDTypeClusterPowerSourceAttributeActiveBatFaultsID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000012,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatReplacementDescriptionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000013,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatCommonDesignationID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000014,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatANSIDesignationID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000015,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatIECDesignationID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000016,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatApprovedChemistryID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000017,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatCapacityID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000018,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatQuantityID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000019,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatChargeStateID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000001A,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatTimeToFullChargeID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000001B,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatFunctionalWhileChargingID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000001C,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatChargingCurrentID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000001D,
    MTRAttributeIDTypeClusterPowerSourceAttributeActiveBatChargeFaultsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000001E,
    MTRAttributeIDTypeClusterPowerSourceAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterPowerSourceAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterPowerSourceAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterPowerSourceAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterPowerSourceAttributeClusterRevisionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster GeneralCommissioning deprecated attribute names
    MTRClusterGeneralCommissioningAttributeBreadcrumbID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeBreadcrumbID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterGeneralCommissioningAttributeBasicCommissioningInfoID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeBasicCommissioningInfoID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterGeneralCommissioningAttributeRegulatoryConfigID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeRegulatoryConfigID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterGeneralCommissioningAttributeLocationCapabilityID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeLocationCapabilityID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterGeneralCommissioningAttributeSupportsConcurrentConnectionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeSupportsConcurrentConnectionID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterGeneralCommissioningAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeGeneratedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterGeneralCommissioningAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeAcceptedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterGeneralCommissioningAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterGeneralCommissioningAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterGeneralCommissioningAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster GeneralCommissioning attributes
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeBreadcrumbID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeBasicCommissioningInfoID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeRegulatoryConfigID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeLocationCapabilityID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeSupportsConcurrentConnectionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeFeatureMapID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster NetworkCommissioning deprecated attribute names
    MTRClusterNetworkCommissioningAttributeMaxNetworksID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeMaxNetworksID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterNetworkCommissioningAttributeNetworksID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeNetworksID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterNetworkCommissioningAttributeScanMaxTimeSecondsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeScanMaxTimeSecondsID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterNetworkCommissioningAttributeConnectMaxTimeSecondsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeConnectMaxTimeSecondsID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterNetworkCommissioningAttributeInterfaceEnabledID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeInterfaceEnabledID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterNetworkCommissioningAttributeLastNetworkingStatusID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeLastNetworkingStatusID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterNetworkCommissioningAttributeLastNetworkIDID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeLastNetworkIDID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterNetworkCommissioningAttributeLastConnectErrorValueID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeLastConnectErrorValueID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterNetworkCommissioningAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeGeneratedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterNetworkCommissioningAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeAcceptedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterNetworkCommissioningAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterNetworkCommissioningAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterNetworkCommissioningAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster NetworkCommissioning attributes
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeMaxNetworksID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeNetworksID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeScanMaxTimeSecondsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeConnectMaxTimeSecondsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeInterfaceEnabledID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeLastNetworkingStatusID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeLastNetworkIDID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeLastConnectErrorValueID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000007,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeFeatureMapID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster DiagnosticLogs deprecated attribute names
    MTRClusterDiagnosticLogsAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDiagnosticLogsAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterDiagnosticLogsAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDiagnosticLogsAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterDiagnosticLogsAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDiagnosticLogsAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterDiagnosticLogsAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDiagnosticLogsAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterDiagnosticLogsAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDiagnosticLogsAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster DiagnosticLogs attributes
    MTRAttributeIDTypeClusterDiagnosticLogsAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterDiagnosticLogsAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterDiagnosticLogsAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterDiagnosticLogsAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterDiagnosticLogsAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster GeneralDiagnostics deprecated attribute names
    MTRClusterGeneralDiagnosticsAttributeNetworkInterfacesID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeNetworkInterfacesID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterGeneralDiagnosticsAttributeRebootCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeRebootCountID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterGeneralDiagnosticsAttributeUpTimeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeUpTimeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterGeneralDiagnosticsAttributeTotalOperationalHoursID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeTotalOperationalHoursID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterGeneralDiagnosticsAttributeBootReasonsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeBootReasonID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterGeneralDiagnosticsAttributeActiveHardwareFaultsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeActiveHardwareFaultsID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterGeneralDiagnosticsAttributeActiveRadioFaultsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeActiveRadioFaultsID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterGeneralDiagnosticsAttributeActiveNetworkFaultsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeActiveNetworkFaultsID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterGeneralDiagnosticsAttributeTestEventTriggersEnabledID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeTestEventTriggersEnabledID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterGeneralDiagnosticsAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterGeneralDiagnosticsAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterGeneralDiagnosticsAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterGeneralDiagnosticsAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterGeneralDiagnosticsAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster GeneralDiagnostics attributes
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeNetworkInterfacesID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeRebootCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeUpTimeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeTotalOperationalHoursID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeBootReasonID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeActiveHardwareFaultsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeActiveRadioFaultsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeActiveNetworkFaultsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000007,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeTestEventTriggersEnabledID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000008,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster SoftwareDiagnostics deprecated attribute names
    MTRClusterSoftwareDiagnosticsAttributeThreadMetricsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeThreadMetricsID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterSoftwareDiagnosticsAttributeCurrentHeapFreeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeCurrentHeapFreeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterSoftwareDiagnosticsAttributeCurrentHeapUsedID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeCurrentHeapUsedID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterSoftwareDiagnosticsAttributeCurrentHeapHighWatermarkID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeCurrentHeapHighWatermarkID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterSoftwareDiagnosticsAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeGeneratedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterSoftwareDiagnosticsAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterSoftwareDiagnosticsAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterSoftwareDiagnosticsAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterSoftwareDiagnosticsAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster SoftwareDiagnostics attributes
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeThreadMetricsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeCurrentHeapFreeID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeCurrentHeapUsedID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeCurrentHeapHighWatermarkID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeFeatureMapID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ThreadNetworkDiagnostics deprecated attribute names
    MTRClusterThreadNetworkDiagnosticsAttributeChannelID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeChannelID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterThreadNetworkDiagnosticsAttributeRoutingRoleID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRoutingRoleID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterThreadNetworkDiagnosticsAttributeNetworkNameID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeNetworkNameID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterThreadNetworkDiagnosticsAttributePanIdID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePanIdID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterThreadNetworkDiagnosticsAttributeExtendedPanIdID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeExtendedPanIdID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterThreadNetworkDiagnosticsAttributeMeshLocalPrefixID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeMeshLocalPrefixID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterThreadNetworkDiagnosticsAttributeOverrunCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeOverrunCountID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterThreadNetworkDiagnosticsAttributeNeighborTableListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeNeighborTableID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterThreadNetworkDiagnosticsAttributeRouteTableListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRouteTableID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterThreadNetworkDiagnosticsAttributePartitionIdID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePartitionIdID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterThreadNetworkDiagnosticsAttributeWeightingID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeWeightingID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterThreadNetworkDiagnosticsAttributeDataVersionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeDataVersionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000B,
    MTRClusterThreadNetworkDiagnosticsAttributeStableDataVersionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeStableDataVersionID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000C,
    MTRClusterThreadNetworkDiagnosticsAttributeLeaderRouterIdID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeLeaderRouterIdID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000D,
    MTRClusterThreadNetworkDiagnosticsAttributeDetachedRoleCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeDetachedRoleCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000E,
    MTRClusterThreadNetworkDiagnosticsAttributeChildRoleCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeChildRoleCountID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000F,
    MTRClusterThreadNetworkDiagnosticsAttributeRouterRoleCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRouterRoleCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,
    MTRClusterThreadNetworkDiagnosticsAttributeLeaderRoleCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeLeaderRoleCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterThreadNetworkDiagnosticsAttributeAttachAttemptCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeAttachAttemptCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterThreadNetworkDiagnosticsAttributePartitionIdChangeCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePartitionIdChangeCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000013,
    MTRClusterThreadNetworkDiagnosticsAttributeBetterPartitionAttachAttemptCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeBetterPartitionAttachAttemptCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000014,
    MTRClusterThreadNetworkDiagnosticsAttributeParentChangeCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeParentChangeCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000015,
    MTRClusterThreadNetworkDiagnosticsAttributeTxTotalCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxTotalCountID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000016,
    MTRClusterThreadNetworkDiagnosticsAttributeTxUnicastCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxUnicastCountID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000017,
    MTRClusterThreadNetworkDiagnosticsAttributeTxBroadcastCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxBroadcastCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000018,
    MTRClusterThreadNetworkDiagnosticsAttributeTxAckRequestedCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxAckRequestedCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000019,
    MTRClusterThreadNetworkDiagnosticsAttributeTxAckedCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxAckedCountID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001A,
    MTRClusterThreadNetworkDiagnosticsAttributeTxNoAckRequestedCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxNoAckRequestedCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001B,
    MTRClusterThreadNetworkDiagnosticsAttributeTxDataCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxDataCountID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001C,
    MTRClusterThreadNetworkDiagnosticsAttributeTxDataPollCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxDataPollCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001D,
    MTRClusterThreadNetworkDiagnosticsAttributeTxBeaconCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxBeaconCountID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001E,
    MTRClusterThreadNetworkDiagnosticsAttributeTxBeaconRequestCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxBeaconRequestCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001F,
    MTRClusterThreadNetworkDiagnosticsAttributeTxOtherCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxOtherCountID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000020,
    MTRClusterThreadNetworkDiagnosticsAttributeTxRetryCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxRetryCountID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000021,
    MTRClusterThreadNetworkDiagnosticsAttributeTxDirectMaxRetryExpiryCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxDirectMaxRetryExpiryCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000022,
    MTRClusterThreadNetworkDiagnosticsAttributeTxIndirectMaxRetryExpiryCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxIndirectMaxRetryExpiryCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000023,
    MTRClusterThreadNetworkDiagnosticsAttributeTxErrCcaCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxErrCcaCountID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000024,
    MTRClusterThreadNetworkDiagnosticsAttributeTxErrAbortCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxErrAbortCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000025,
    MTRClusterThreadNetworkDiagnosticsAttributeTxErrBusyChannelCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxErrBusyChannelCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000026,
    MTRClusterThreadNetworkDiagnosticsAttributeRxTotalCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxTotalCountID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000027,
    MTRClusterThreadNetworkDiagnosticsAttributeRxUnicastCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxUnicastCountID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000028,
    MTRClusterThreadNetworkDiagnosticsAttributeRxBroadcastCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxBroadcastCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000029,
    MTRClusterThreadNetworkDiagnosticsAttributeRxDataCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDataCountID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002A,
    MTRClusterThreadNetworkDiagnosticsAttributeRxDataPollCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDataPollCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002B,
    MTRClusterThreadNetworkDiagnosticsAttributeRxBeaconCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxBeaconCountID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002C,
    MTRClusterThreadNetworkDiagnosticsAttributeRxBeaconRequestCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxBeaconRequestCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002D,
    MTRClusterThreadNetworkDiagnosticsAttributeRxOtherCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxOtherCountID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002E,
    MTRClusterThreadNetworkDiagnosticsAttributeRxAddressFilteredCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxAddressFilteredCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002F,
    MTRClusterThreadNetworkDiagnosticsAttributeRxDestAddrFilteredCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDestAddrFilteredCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000030,
    MTRClusterThreadNetworkDiagnosticsAttributeRxDuplicatedCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDuplicatedCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000031,
    MTRClusterThreadNetworkDiagnosticsAttributeRxErrNoFrameCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrNoFrameCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000032,
    MTRClusterThreadNetworkDiagnosticsAttributeRxErrUnknownNeighborCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrUnknownNeighborCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000033,
    MTRClusterThreadNetworkDiagnosticsAttributeRxErrInvalidSrcAddrCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrInvalidSrcAddrCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000034,
    MTRClusterThreadNetworkDiagnosticsAttributeRxErrSecCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrSecCountID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000035,
    MTRClusterThreadNetworkDiagnosticsAttributeRxErrFcsCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrFcsCountID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000036,
    MTRClusterThreadNetworkDiagnosticsAttributeRxErrOtherCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrOtherCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000037,
    MTRClusterThreadNetworkDiagnosticsAttributeActiveTimestampID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeActiveTimestampID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000038,
    MTRClusterThreadNetworkDiagnosticsAttributePendingTimestampID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePendingTimestampID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000039,
    MTRClusterThreadNetworkDiagnosticsAttributeDelayID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeDelayID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000003A,
    MTRClusterThreadNetworkDiagnosticsAttributeSecurityPolicyID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeSecurityPolicyID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000003B,
    MTRClusterThreadNetworkDiagnosticsAttributeChannelPage0MaskID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeChannelPage0MaskID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000003C,
    MTRClusterThreadNetworkDiagnosticsAttributeOperationalDatasetComponentsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeOperationalDatasetComponentsID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000003D,
    MTRClusterThreadNetworkDiagnosticsAttributeActiveNetworkFaultsListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeActiveNetworkFaultsListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000003E,
    MTRClusterThreadNetworkDiagnosticsAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeGeneratedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterThreadNetworkDiagnosticsAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeAcceptedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterThreadNetworkDiagnosticsAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterThreadNetworkDiagnosticsAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterThreadNetworkDiagnosticsAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeClusterRevisionID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ThreadNetworkDiagnostics attributes
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeChannelID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRoutingRoleID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeNetworkNameID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePanIdID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeExtendedPanIdID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeMeshLocalPrefixID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeOverrunCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeNeighborTableID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000007,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRouteTableID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000008,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePartitionIdID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000009,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeWeightingID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000A,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeDataVersionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000B,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeStableDataVersionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000C,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeLeaderRouterIdID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000D,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeDetachedRoleCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000E,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeChildRoleCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000F,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRouterRoleCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000010,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeLeaderRoleCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000011,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeAttachAttemptCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000012,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePartitionIdChangeCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000013,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeBetterPartitionAttachAttemptCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000014,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeParentChangeCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000015,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxTotalCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000016,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxUnicastCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000017,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxBroadcastCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000018,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxAckRequestedCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000019,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxAckedCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000001A,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxNoAckRequestedCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000001B,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxDataCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000001C,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxDataPollCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000001D,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxBeaconCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000001E,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxBeaconRequestCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000001F,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxOtherCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000020,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxRetryCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000021,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxDirectMaxRetryExpiryCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000022,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxIndirectMaxRetryExpiryCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000023,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxErrCcaCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000024,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxErrAbortCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000025,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxErrBusyChannelCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000026,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxTotalCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000027,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxUnicastCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000028,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxBroadcastCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000029,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDataCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000002A,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDataPollCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000002B,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxBeaconCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000002C,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxBeaconRequestCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000002D,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxOtherCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000002E,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxAddressFilteredCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000002F,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDestAddrFilteredCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000030,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDuplicatedCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000031,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrNoFrameCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000032,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrUnknownNeighborCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000033,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrInvalidSrcAddrCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000034,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrSecCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000035,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrFcsCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000036,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrOtherCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000037,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeActiveTimestampID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000038,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePendingTimestampID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000039,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeDelayID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000003A,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeSecurityPolicyID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000003B,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeChannelPage0MaskID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000003C,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeOperationalDatasetComponentsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000003D,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeActiveNetworkFaultsListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000003E,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeFeatureMapID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster WiFiNetworkDiagnostics deprecated attribute names
    MTRClusterWiFiNetworkDiagnosticsAttributeBssidID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeBSSIDID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterWiFiNetworkDiagnosticsAttributeSecurityTypeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeSecurityTypeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterWiFiNetworkDiagnosticsAttributeWiFiVersionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeWiFiVersionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterWiFiNetworkDiagnosticsAttributeChannelNumberID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeChannelNumberID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterWiFiNetworkDiagnosticsAttributeRssiID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeRSSIID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterWiFiNetworkDiagnosticsAttributeBeaconLostCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeBeaconLostCountID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterWiFiNetworkDiagnosticsAttributeBeaconRxCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeBeaconRxCountID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterWiFiNetworkDiagnosticsAttributePacketMulticastRxCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketMulticastRxCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterWiFiNetworkDiagnosticsAttributePacketMulticastTxCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketMulticastTxCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterWiFiNetworkDiagnosticsAttributePacketUnicastRxCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketUnicastRxCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterWiFiNetworkDiagnosticsAttributePacketUnicastTxCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketUnicastTxCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterWiFiNetworkDiagnosticsAttributeCurrentMaxRateID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeCurrentMaxRateID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000B,
    MTRClusterWiFiNetworkDiagnosticsAttributeOverrunCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeOverrunCountID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000C,
    MTRClusterWiFiNetworkDiagnosticsAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeGeneratedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterWiFiNetworkDiagnosticsAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeAcceptedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterWiFiNetworkDiagnosticsAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterWiFiNetworkDiagnosticsAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterWiFiNetworkDiagnosticsAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster WiFiNetworkDiagnostics attributes
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeBSSIDID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeSecurityTypeID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeWiFiVersionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeChannelNumberID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeRSSIID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeBeaconLostCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeBeaconRxCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketMulticastRxCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000007,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketMulticastTxCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000008,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketUnicastRxCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000009,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketUnicastTxCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000A,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeCurrentMaxRateID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000B,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeOverrunCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000C,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeFeatureMapID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster EthernetNetworkDiagnostics deprecated attribute names
    MTRClusterEthernetNetworkDiagnosticsAttributePHYRateID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributePHYRateID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterEthernetNetworkDiagnosticsAttributeFullDuplexID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeFullDuplexID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterEthernetNetworkDiagnosticsAttributePacketRxCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributePacketRxCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterEthernetNetworkDiagnosticsAttributePacketTxCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributePacketTxCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterEthernetNetworkDiagnosticsAttributeTxErrCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeTxErrCountID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterEthernetNetworkDiagnosticsAttributeCollisionCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeCollisionCountID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterEthernetNetworkDiagnosticsAttributeOverrunCountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeOverrunCountID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterEthernetNetworkDiagnosticsAttributeCarrierDetectID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeCarrierDetectID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterEthernetNetworkDiagnosticsAttributeTimeSinceResetID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeTimeSinceResetID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterEthernetNetworkDiagnosticsAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeGeneratedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterEthernetNetworkDiagnosticsAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeAcceptedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterEthernetNetworkDiagnosticsAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeAttributeListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterEthernetNetworkDiagnosticsAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterEthernetNetworkDiagnosticsAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeClusterRevisionID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster EthernetNetworkDiagnostics attributes
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributePHYRateID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeFullDuplexID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributePacketRxCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributePacketTxCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeTxErrCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeCollisionCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeOverrunCountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeCarrierDetectID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000007,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeTimeSinceResetID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000008,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeFeatureMapID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster TimeSynchronization deprecated attribute names
    MTRClusterTimeSynchronizationAttributeUTCTimeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeUTCTimeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterTimeSynchronizationAttributeGranularityID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeGranularityID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterTimeSynchronizationAttributeTimeSourceID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeTimeSourceID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterTimeSynchronizationAttributeTrustedTimeNodeIdID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeTrustedTimeNodeIdID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterTimeSynchronizationAttributeDefaultNtpID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeDefaultNtpID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterTimeSynchronizationAttributeTimeZoneID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeTimeZoneID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterTimeSynchronizationAttributeDstOffsetID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeDstOffsetID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterTimeSynchronizationAttributeLocalTimeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeLocalTimeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterTimeSynchronizationAttributeTimeZoneDatabaseID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeTimeZoneDatabaseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterTimeSynchronizationAttributeNtpServerPortID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeNtpServerPortID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterTimeSynchronizationAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeGeneratedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterTimeSynchronizationAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterTimeSynchronizationAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterTimeSynchronizationAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterTimeSynchronizationAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster TimeSynchronization attributes
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeUTCTimeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeGranularityID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeTimeSourceID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeTrustedTimeNodeIdID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeDefaultNtpID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeTimeZoneID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeDstOffsetID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeLocalTimeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000007,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeTimeZoneDatabaseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000008,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeNtpServerPortID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000009,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeFeatureMapID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster BridgedDeviceBasic deprecated attribute names
    MTRClusterBridgedDeviceBasicAttributeVendorNameID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeVendorNameID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterBridgedDeviceBasicAttributeVendorIDID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeVendorIDID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterBridgedDeviceBasicAttributeProductNameID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeProductNameID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterBridgedDeviceBasicAttributeNodeLabelID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeNodeLabelID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterBridgedDeviceBasicAttributeHardwareVersionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeHardwareVersionID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterBridgedDeviceBasicAttributeHardwareVersionStringID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeHardwareVersionStringID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterBridgedDeviceBasicAttributeSoftwareVersionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeSoftwareVersionID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterBridgedDeviceBasicAttributeSoftwareVersionStringID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeSoftwareVersionStringID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterBridgedDeviceBasicAttributeManufacturingDateID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeManufacturingDateID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000B,
    MTRClusterBridgedDeviceBasicAttributePartNumberID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributePartNumberID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000C,
    MTRClusterBridgedDeviceBasicAttributeProductURLID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeProductURLID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000D,
    MTRClusterBridgedDeviceBasicAttributeProductLabelID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeProductLabelID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000E,
    MTRClusterBridgedDeviceBasicAttributeSerialNumberID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeSerialNumberID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000F,
    MTRClusterBridgedDeviceBasicAttributeReachableID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeReachableID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterBridgedDeviceBasicAttributeUniqueIDID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeUniqueIDID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterBridgedDeviceBasicAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeGeneratedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterBridgedDeviceBasicAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeAcceptedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterBridgedDeviceBasicAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeAttributeListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterBridgedDeviceBasicAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeFeatureMapID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterBridgedDeviceBasicAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeClusterRevisionID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster BridgedDeviceBasicInformation attributes
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeVendorNameID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeVendorIDID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeProductNameID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeNodeLabelID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeHardwareVersionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000007,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeHardwareVersionStringID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000008,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeSoftwareVersionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000009,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeSoftwareVersionStringID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000A,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeManufacturingDateID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000B,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributePartNumberID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000C,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeProductURLID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000D,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeProductLabelID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000E,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeSerialNumberID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000F,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeReachableID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000011,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeUniqueIDID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000012,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeFeatureMapID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Switch deprecated attribute names
    MTRClusterSwitchAttributeNumberOfPositionsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterSwitchAttributeNumberOfPositionsID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterSwitchAttributeCurrentPositionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterSwitchAttributeCurrentPositionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterSwitchAttributeMultiPressMaxID API_DEPRECATED("Please use MTRAttributeIDTypeClusterSwitchAttributeMultiPressMaxID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterSwitchAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterSwitchAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterSwitchAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterSwitchAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterSwitchAttributeAttributeListID API_DEPRECATED("Please use MTRAttributeIDTypeClusterSwitchAttributeAttributeListID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterSwitchAttributeFeatureMapID API_DEPRECATED("Please use MTRAttributeIDTypeClusterSwitchAttributeFeatureMapID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterSwitchAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterSwitchAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster Switch attributes
    MTRAttributeIDTypeClusterSwitchAttributeNumberOfPositionsID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterSwitchAttributeCurrentPositionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterSwitchAttributeMultiPressMaxID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterSwitchAttributeGeneratedCommandListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterSwitchAttributeAcceptedCommandListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterSwitchAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterSwitchAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterSwitchAttributeClusterRevisionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster AdministratorCommissioning deprecated attribute names
    MTRClusterAdministratorCommissioningAttributeWindowStatusID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeWindowStatusID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterAdministratorCommissioningAttributeAdminFabricIndexID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAdminFabricIndexID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterAdministratorCommissioningAttributeAdminVendorIdID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAdminVendorIdID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterAdministratorCommissioningAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeGeneratedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterAdministratorCommissioningAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAcceptedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterAdministratorCommissioningAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAttributeListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterAdministratorCommissioningAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterAdministratorCommissioningAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeClusterRevisionID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster AdministratorCommissioning attributes
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeWindowStatusID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAdminFabricIndexID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAdminVendorIdID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeFeatureMapID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster OperationalCredentials deprecated attribute names
    MTRClusterOperationalCredentialsAttributeNOCsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeNOCsID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterOperationalCredentialsAttributeFabricsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeFabricsID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterOperationalCredentialsAttributeSupportedFabricsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeSupportedFabricsID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterOperationalCredentialsAttributeCommissionedFabricsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeCommissionedFabricsID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterOperationalCredentialsAttributeTrustedRootCertificatesID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeTrustedRootCertificatesID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterOperationalCredentialsAttributeCurrentFabricIndexID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeCurrentFabricIndexID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterOperationalCredentialsAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeGeneratedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterOperationalCredentialsAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeAcceptedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterOperationalCredentialsAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterOperationalCredentialsAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterOperationalCredentialsAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster OperationalCredentials attributes
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeNOCsID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeFabricsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeSupportedFabricsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeCommissionedFabricsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeTrustedRootCertificatesID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeCurrentFabricIndexID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeFeatureMapID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster GroupKeyManagement deprecated attribute names
    MTRClusterGroupKeyManagementAttributeGroupKeyMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeGroupKeyMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterGroupKeyManagementAttributeGroupTableID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeGroupTableID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterGroupKeyManagementAttributeMaxGroupsPerFabricID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeMaxGroupsPerFabricID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterGroupKeyManagementAttributeMaxGroupKeysPerFabricID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeMaxGroupKeysPerFabricID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterGroupKeyManagementAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterGroupKeyManagementAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterGroupKeyManagementAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterGroupKeyManagementAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterGroupKeyManagementAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster GroupKeyManagement attributes
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeGroupKeyMapID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeGroupTableID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeMaxGroupsPerFabricID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeMaxGroupKeysPerFabricID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster FixedLabel deprecated attribute names
    MTRClusterFixedLabelAttributeLabelListID API_DEPRECATED("Please use MTRAttributeIDTypeClusterFixedLabelAttributeLabelListID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterFixedLabelAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFixedLabelAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterFixedLabelAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFixedLabelAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterFixedLabelAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFixedLabelAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterFixedLabelAttributeFeatureMapID API_DEPRECATED("Please use MTRAttributeIDTypeClusterFixedLabelAttributeFeatureMapID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterFixedLabelAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFixedLabelAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster FixedLabel attributes
    MTRAttributeIDTypeClusterFixedLabelAttributeLabelListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterFixedLabelAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterFixedLabelAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterFixedLabelAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterFixedLabelAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterFixedLabelAttributeClusterRevisionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster UserLabel deprecated attribute names
    MTRClusterUserLabelAttributeLabelListID API_DEPRECATED("Please use MTRAttributeIDTypeClusterUserLabelAttributeLabelListID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterUserLabelAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUserLabelAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterUserLabelAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUserLabelAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterUserLabelAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUserLabelAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterUserLabelAttributeFeatureMapID API_DEPRECATED("Please use MTRAttributeIDTypeClusterUserLabelAttributeFeatureMapID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterUserLabelAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUserLabelAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster UserLabel attributes
    MTRAttributeIDTypeClusterUserLabelAttributeLabelListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterUserLabelAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterUserLabelAttributeAcceptedCommandListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterUserLabelAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterUserLabelAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterUserLabelAttributeClusterRevisionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ProxyConfiguration deprecated attribute names
    MTRClusterProxyConfigurationAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterProxyConfigurationAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterProxyConfigurationAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterProxyConfigurationAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterProxyConfigurationAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterProxyConfigurationAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterProxyConfigurationAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterProxyConfigurationAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterProxyConfigurationAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterProxyConfigurationAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ProxyConfiguration attributes
    MTRAttributeIDTypeClusterProxyConfigurationAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterProxyConfigurationAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterProxyConfigurationAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterProxyConfigurationAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterProxyConfigurationAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ProxyDiscovery deprecated attribute names
    MTRClusterProxyDiscoveryAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterProxyDiscoveryAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterProxyDiscoveryAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterProxyDiscoveryAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterProxyDiscoveryAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterProxyDiscoveryAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterProxyDiscoveryAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterProxyDiscoveryAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterProxyDiscoveryAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterProxyDiscoveryAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ProxyDiscovery attributes
    MTRAttributeIDTypeClusterProxyDiscoveryAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterProxyDiscoveryAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterProxyDiscoveryAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterProxyDiscoveryAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterProxyDiscoveryAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ProxyValid deprecated attribute names
    MTRClusterProxyValidAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterProxyValidAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterProxyValidAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterProxyValidAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterProxyValidAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterProxyValidAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterProxyValidAttributeFeatureMapID API_DEPRECATED("Please use MTRAttributeIDTypeClusterProxyValidAttributeFeatureMapID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterProxyValidAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterProxyValidAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ProxyValid attributes
    MTRAttributeIDTypeClusterProxyValidAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterProxyValidAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterProxyValidAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterProxyValidAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterProxyValidAttributeClusterRevisionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster BooleanState deprecated attribute names
    MTRClusterBooleanStateAttributeStateValueID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBooleanStateAttributeStateValueID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterBooleanStateAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBooleanStateAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterBooleanStateAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBooleanStateAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterBooleanStateAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBooleanStateAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterBooleanStateAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBooleanStateAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterBooleanStateAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBooleanStateAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster BooleanState attributes
    MTRAttributeIDTypeClusterBooleanStateAttributeStateValueID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterBooleanStateAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBooleanStateAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBooleanStateAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBooleanStateAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBooleanStateAttributeClusterRevisionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ModeSelect deprecated attribute names
    MTRClusterModeSelectAttributeDescriptionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterModeSelectAttributeDescriptionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterModeSelectAttributeStandardNamespaceID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterModeSelectAttributeStandardNamespaceID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterModeSelectAttributeSupportedModesID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterModeSelectAttributeSupportedModesID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterModeSelectAttributeCurrentModeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterModeSelectAttributeCurrentModeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterModeSelectAttributeStartUpModeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterModeSelectAttributeStartUpModeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterModeSelectAttributeOnModeID API_DEPRECATED("Please use MTRAttributeIDTypeClusterModeSelectAttributeOnModeID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterModeSelectAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterModeSelectAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterModeSelectAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterModeSelectAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterModeSelectAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterModeSelectAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterModeSelectAttributeFeatureMapID API_DEPRECATED("Please use MTRAttributeIDTypeClusterModeSelectAttributeFeatureMapID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterModeSelectAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterModeSelectAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ModeSelect attributes
    MTRAttributeIDTypeClusterModeSelectAttributeDescriptionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterModeSelectAttributeStandardNamespaceID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterModeSelectAttributeSupportedModesID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterModeSelectAttributeCurrentModeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterModeSelectAttributeStartUpModeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRAttributeIDTypeClusterModeSelectAttributeOnModeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTRAttributeIDTypeClusterModeSelectAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterModeSelectAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterModeSelectAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterModeSelectAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterModeSelectAttributeClusterRevisionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster DoorLock deprecated attribute names
    MTRClusterDoorLockAttributeLockStateID API_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeLockStateID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterDoorLockAttributeLockTypeID API_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeLockTypeID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterDoorLockAttributeActuatorEnabledID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeActuatorEnabledID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterDoorLockAttributeDoorStateID API_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeDoorStateID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterDoorLockAttributeDoorOpenEventsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeDoorOpenEventsID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterDoorLockAttributeDoorClosedEventsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeDoorClosedEventsID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterDoorLockAttributeOpenPeriodID API_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeOpenPeriodID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterDoorLockAttributeNumberOfTotalUsersSupportedID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeNumberOfTotalUsersSupportedID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterDoorLockAttributeNumberOfPINUsersSupportedID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeNumberOfPINUsersSupportedID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterDoorLockAttributeNumberOfRFIDUsersSupportedID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeNumberOfRFIDUsersSupportedID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000013,
    MTRClusterDoorLockAttributeNumberOfWeekDaySchedulesSupportedPerUserID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeNumberOfWeekDaySchedulesSupportedPerUserID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000014,
    MTRClusterDoorLockAttributeNumberOfYearDaySchedulesSupportedPerUserID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeNumberOfYearDaySchedulesSupportedPerUserID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000015,
    MTRClusterDoorLockAttributeNumberOfHolidaySchedulesSupportedID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeNumberOfHolidaySchedulesSupportedID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000016,
    MTRClusterDoorLockAttributeMaxPINCodeLengthID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeMaxPINCodeLengthID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000017,
    MTRClusterDoorLockAttributeMinPINCodeLengthID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeMinPINCodeLengthID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000018,
    MTRClusterDoorLockAttributeMaxRFIDCodeLengthID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeMaxRFIDCodeLengthID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000019,
    MTRClusterDoorLockAttributeMinRFIDCodeLengthID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeMinRFIDCodeLengthID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001A,
    MTRClusterDoorLockAttributeCredentialRulesSupportID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeCredentialRulesSupportID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001B,
    MTRClusterDoorLockAttributeNumberOfCredentialsSupportedPerUserID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeNumberOfCredentialsSupportedPerUserID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001C,
    MTRClusterDoorLockAttributeLanguageID API_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeLanguageID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000021,
    MTRClusterDoorLockAttributeLEDSettingsID API_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeLEDSettingsID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000022,
    MTRClusterDoorLockAttributeAutoRelockTimeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeAutoRelockTimeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000023,
    MTRClusterDoorLockAttributeSoundVolumeID API_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeSoundVolumeID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000024,
    MTRClusterDoorLockAttributeOperatingModeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeOperatingModeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000025,
    MTRClusterDoorLockAttributeSupportedOperatingModesID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeSupportedOperatingModesID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000026,
    MTRClusterDoorLockAttributeDefaultConfigurationRegisterID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeDefaultConfigurationRegisterID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000027,
    MTRClusterDoorLockAttributeEnableLocalProgrammingID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeEnableLocalProgrammingID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000028,
    MTRClusterDoorLockAttributeEnableOneTouchLockingID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeEnableOneTouchLockingID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000029,
    MTRClusterDoorLockAttributeEnableInsideStatusLEDID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeEnableInsideStatusLEDID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002A,
    MTRClusterDoorLockAttributeEnablePrivacyModeButtonID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeEnablePrivacyModeButtonID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002B,
    MTRClusterDoorLockAttributeLocalProgrammingFeaturesID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeLocalProgrammingFeaturesID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002C,
    MTRClusterDoorLockAttributeWrongCodeEntryLimitID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeWrongCodeEntryLimitID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000030,
    MTRClusterDoorLockAttributeUserCodeTemporaryDisableTimeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeUserCodeTemporaryDisableTimeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000031,
    MTRClusterDoorLockAttributeSendPINOverTheAirID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeSendPINOverTheAirID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000032,
    MTRClusterDoorLockAttributeRequirePINforRemoteOperationID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeRequirePINforRemoteOperationID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000033,
    MTRClusterDoorLockAttributeExpiringUserTimeoutID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeExpiringUserTimeoutID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000035,
    MTRClusterDoorLockAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterDoorLockAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterDoorLockAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterDoorLockAttributeFeatureMapID API_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeFeatureMapID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterDoorLockAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster DoorLock attributes
    MTRAttributeIDTypeClusterDoorLockAttributeLockStateID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterDoorLockAttributeLockTypeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterDoorLockAttributeActuatorEnabledID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterDoorLockAttributeDoorStateID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterDoorLockAttributeDoorOpenEventsID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRAttributeIDTypeClusterDoorLockAttributeDoorClosedEventsID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTRAttributeIDTypeClusterDoorLockAttributeOpenPeriodID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,
    MTRAttributeIDTypeClusterDoorLockAttributeNumberOfTotalUsersSupportedID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000011,
    MTRAttributeIDTypeClusterDoorLockAttributeNumberOfPINUsersSupportedID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000012,
    MTRAttributeIDTypeClusterDoorLockAttributeNumberOfRFIDUsersSupportedID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000013,
    MTRAttributeIDTypeClusterDoorLockAttributeNumberOfWeekDaySchedulesSupportedPerUserID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000014,
    MTRAttributeIDTypeClusterDoorLockAttributeNumberOfYearDaySchedulesSupportedPerUserID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000015,
    MTRAttributeIDTypeClusterDoorLockAttributeNumberOfHolidaySchedulesSupportedID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000016,
    MTRAttributeIDTypeClusterDoorLockAttributeMaxPINCodeLengthID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000017,
    MTRAttributeIDTypeClusterDoorLockAttributeMinPINCodeLengthID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000018,
    MTRAttributeIDTypeClusterDoorLockAttributeMaxRFIDCodeLengthID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000019,
    MTRAttributeIDTypeClusterDoorLockAttributeMinRFIDCodeLengthID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000001A,
    MTRAttributeIDTypeClusterDoorLockAttributeCredentialRulesSupportID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000001B,
    MTRAttributeIDTypeClusterDoorLockAttributeNumberOfCredentialsSupportedPerUserID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000001C,
    MTRAttributeIDTypeClusterDoorLockAttributeLanguageID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000021,
    MTRAttributeIDTypeClusterDoorLockAttributeLEDSettingsID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000022,
    MTRAttributeIDTypeClusterDoorLockAttributeAutoRelockTimeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000023,
    MTRAttributeIDTypeClusterDoorLockAttributeSoundVolumeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000024,
    MTRAttributeIDTypeClusterDoorLockAttributeOperatingModeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000025,
    MTRAttributeIDTypeClusterDoorLockAttributeSupportedOperatingModesID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000026,
    MTRAttributeIDTypeClusterDoorLockAttributeDefaultConfigurationRegisterID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000027,
    MTRAttributeIDTypeClusterDoorLockAttributeEnableLocalProgrammingID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000028,
    MTRAttributeIDTypeClusterDoorLockAttributeEnableOneTouchLockingID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000029,
    MTRAttributeIDTypeClusterDoorLockAttributeEnableInsideStatusLEDID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000002A,
    MTRAttributeIDTypeClusterDoorLockAttributeEnablePrivacyModeButtonID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000002B,
    MTRAttributeIDTypeClusterDoorLockAttributeLocalProgrammingFeaturesID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000002C,
    MTRAttributeIDTypeClusterDoorLockAttributeWrongCodeEntryLimitID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000030,
    MTRAttributeIDTypeClusterDoorLockAttributeUserCodeTemporaryDisableTimeID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000031,
    MTRAttributeIDTypeClusterDoorLockAttributeSendPINOverTheAirID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000032,
    MTRAttributeIDTypeClusterDoorLockAttributeRequirePINforRemoteOperationID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000033,
    MTRAttributeIDTypeClusterDoorLockAttributeExpiringUserTimeoutID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000035,
    MTRAttributeIDTypeClusterDoorLockAttributeGeneratedCommandListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterDoorLockAttributeAcceptedCommandListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterDoorLockAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterDoorLockAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterDoorLockAttributeClusterRevisionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster WindowCovering deprecated attribute names
    MTRClusterWindowCoveringAttributeTypeID API_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeTypeID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterWindowCoveringAttributePhysicalClosedLimitLiftID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributePhysicalClosedLimitLiftID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterWindowCoveringAttributePhysicalClosedLimitTiltID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributePhysicalClosedLimitTiltID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterWindowCoveringAttributeCurrentPositionLiftID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionLiftID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterWindowCoveringAttributeCurrentPositionTiltID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionTiltID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterWindowCoveringAttributeNumberOfActuationsLiftID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeNumberOfActuationsLiftID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterWindowCoveringAttributeNumberOfActuationsTiltID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeNumberOfActuationsTiltID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterWindowCoveringAttributeConfigStatusID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeConfigStatusID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterWindowCoveringAttributeCurrentPositionLiftPercentageID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionLiftPercentageID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterWindowCoveringAttributeCurrentPositionTiltPercentageID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionTiltPercentageID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterWindowCoveringAttributeOperationalStatusID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeOperationalStatusID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterWindowCoveringAttributeTargetPositionLiftPercent100thsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeTargetPositionLiftPercent100thsID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000B,
    MTRClusterWindowCoveringAttributeTargetPositionTiltPercent100thsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeTargetPositionTiltPercent100thsID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000C,
    MTRClusterWindowCoveringAttributeEndProductTypeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeEndProductTypeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000D,
    MTRClusterWindowCoveringAttributeCurrentPositionLiftPercent100thsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionLiftPercent100thsID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000E,
    MTRClusterWindowCoveringAttributeCurrentPositionTiltPercent100thsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionTiltPercent100thsID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000F,
    MTRClusterWindowCoveringAttributeInstalledOpenLimitLiftID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledOpenLimitLiftID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,
    MTRClusterWindowCoveringAttributeInstalledClosedLimitLiftID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledClosedLimitLiftID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterWindowCoveringAttributeInstalledOpenLimitTiltID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledOpenLimitTiltID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterWindowCoveringAttributeInstalledClosedLimitTiltID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledClosedLimitTiltID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000013,
    MTRClusterWindowCoveringAttributeModeID API_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeModeID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000017,
    MTRClusterWindowCoveringAttributeSafetyStatusID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeSafetyStatusID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001A,
    MTRClusterWindowCoveringAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterWindowCoveringAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterWindowCoveringAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterWindowCoveringAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterWindowCoveringAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster WindowCovering attributes
    MTRAttributeIDTypeClusterWindowCoveringAttributeTypeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterWindowCoveringAttributePhysicalClosedLimitLiftID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterWindowCoveringAttributePhysicalClosedLimitTiltID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionLiftID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionTiltID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRAttributeIDTypeClusterWindowCoveringAttributeNumberOfActuationsLiftID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTRAttributeIDTypeClusterWindowCoveringAttributeNumberOfActuationsTiltID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,
    MTRAttributeIDTypeClusterWindowCoveringAttributeConfigStatusID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000007,
    MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionLiftPercentageID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000008,
    MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionTiltPercentageID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000009,
    MTRAttributeIDTypeClusterWindowCoveringAttributeOperationalStatusID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000A,
    MTRAttributeIDTypeClusterWindowCoveringAttributeTargetPositionLiftPercent100thsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000B,
    MTRAttributeIDTypeClusterWindowCoveringAttributeTargetPositionTiltPercent100thsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000C,
    MTRAttributeIDTypeClusterWindowCoveringAttributeEndProductTypeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000D,
    MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionLiftPercent100thsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000E,
    MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionTiltPercent100thsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000F,
    MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledOpenLimitLiftID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000010,
    MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledClosedLimitLiftID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000011,
    MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledOpenLimitTiltID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000012,
    MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledClosedLimitTiltID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000013,
    MTRAttributeIDTypeClusterWindowCoveringAttributeModeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000017,
    MTRAttributeIDTypeClusterWindowCoveringAttributeSafetyStatusID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000001A,
    MTRAttributeIDTypeClusterWindowCoveringAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterWindowCoveringAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterWindowCoveringAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterWindowCoveringAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterWindowCoveringAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster BarrierControl deprecated attribute names
    MTRClusterBarrierControlAttributeBarrierMovingStateID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierMovingStateID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterBarrierControlAttributeBarrierSafetyStatusID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierSafetyStatusID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterBarrierControlAttributeBarrierCapabilitiesID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCapabilitiesID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterBarrierControlAttributeBarrierOpenEventsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierOpenEventsID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterBarrierControlAttributeBarrierCloseEventsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCloseEventsID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterBarrierControlAttributeBarrierCommandOpenEventsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCommandOpenEventsID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterBarrierControlAttributeBarrierCommandCloseEventsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCommandCloseEventsID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterBarrierControlAttributeBarrierOpenPeriodID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierOpenPeriodID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterBarrierControlAttributeBarrierClosePeriodID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierClosePeriodID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterBarrierControlAttributeBarrierPositionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierPositionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterBarrierControlAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBarrierControlAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterBarrierControlAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBarrierControlAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterBarrierControlAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBarrierControlAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterBarrierControlAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBarrierControlAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterBarrierControlAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBarrierControlAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster BarrierControl attributes
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierMovingStateID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierSafetyStatusID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCapabilitiesID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierOpenEventsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCloseEventsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCommandOpenEventsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCommandCloseEventsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000007,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierOpenPeriodID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000008,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierClosePeriodID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000009,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierPositionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000A,
    MTRAttributeIDTypeClusterBarrierControlAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBarrierControlAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBarrierControlAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBarrierControlAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBarrierControlAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster PumpConfigurationAndControl deprecated attribute names
    MTRClusterPumpConfigurationAndControlAttributeMaxPressureID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxPressureID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterPumpConfigurationAndControlAttributeMaxSpeedID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxSpeedID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterPumpConfigurationAndControlAttributeMaxFlowID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxFlowID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterPumpConfigurationAndControlAttributeMinConstPressureID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstPressureID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterPumpConfigurationAndControlAttributeMaxConstPressureID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstPressureID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterPumpConfigurationAndControlAttributeMinCompPressureID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinCompPressureID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterPumpConfigurationAndControlAttributeMaxCompPressureID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxCompPressureID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterPumpConfigurationAndControlAttributeMinConstSpeedID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstSpeedID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterPumpConfigurationAndControlAttributeMaxConstSpeedID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstSpeedID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterPumpConfigurationAndControlAttributeMinConstFlowID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstFlowID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterPumpConfigurationAndControlAttributeMaxConstFlowID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstFlowID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterPumpConfigurationAndControlAttributeMinConstTempID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstTempID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000B,
    MTRClusterPumpConfigurationAndControlAttributeMaxConstTempID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstTempID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000C,
    MTRClusterPumpConfigurationAndControlAttributePumpStatusID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributePumpStatusID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,
    MTRClusterPumpConfigurationAndControlAttributeEffectiveOperationModeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeEffectiveOperationModeID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterPumpConfigurationAndControlAttributeEffectiveControlModeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeEffectiveControlModeID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterPumpConfigurationAndControlAttributeCapacityID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeCapacityID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000013,
    MTRClusterPumpConfigurationAndControlAttributeSpeedID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeSpeedID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000014,
    MTRClusterPumpConfigurationAndControlAttributeLifetimeRunningHoursID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeLifetimeRunningHoursID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000015,
    MTRClusterPumpConfigurationAndControlAttributePowerID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributePowerID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000016,
    MTRClusterPumpConfigurationAndControlAttributeLifetimeEnergyConsumedID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeLifetimeEnergyConsumedID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000017,
    MTRClusterPumpConfigurationAndControlAttributeOperationModeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeOperationModeID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000020,
    MTRClusterPumpConfigurationAndControlAttributeControlModeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeControlModeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000021,
    MTRClusterPumpConfigurationAndControlAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeGeneratedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterPumpConfigurationAndControlAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeAcceptedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterPumpConfigurationAndControlAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeAttributeListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterPumpConfigurationAndControlAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterPumpConfigurationAndControlAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeClusterRevisionID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster PumpConfigurationAndControl attributes
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxPressureID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxSpeedID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxFlowID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstPressureID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstPressureID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinCompPressureID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxCompPressureID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstSpeedID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000007,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstSpeedID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000008,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstFlowID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000009,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstFlowID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000A,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstTempID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000B,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstTempID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000C,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributePumpStatusID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000010,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeEffectiveOperationModeID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000011,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeEffectiveControlModeID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000012,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeCapacityID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000013,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeSpeedID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000014,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeLifetimeRunningHoursID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000015,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributePowerID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000016,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeLifetimeEnergyConsumedID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000017,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeOperationModeID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000020,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeControlModeID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000021,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeFeatureMapID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Thermostat deprecated attribute names
    MTRClusterThermostatAttributeLocalTemperatureID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeLocalTemperatureID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterThermostatAttributeOutdoorTemperatureID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeOutdoorTemperatureID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterThermostatAttributeOccupancyID API_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeOccupancyID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterThermostatAttributeAbsMinHeatSetpointLimitID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeAbsMinHeatSetpointLimitID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterThermostatAttributeAbsMaxHeatSetpointLimitID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeAbsMaxHeatSetpointLimitID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterThermostatAttributeAbsMinCoolSetpointLimitID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeAbsMinCoolSetpointLimitID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterThermostatAttributeAbsMaxCoolSetpointLimitID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeAbsMaxCoolSetpointLimitID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterThermostatAttributePICoolingDemandID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributePICoolingDemandID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterThermostatAttributePIHeatingDemandID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributePIHeatingDemandID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterThermostatAttributeHVACSystemTypeConfigurationID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeHVACSystemTypeConfigurationID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterThermostatAttributeLocalTemperatureCalibrationID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeLocalTemperatureCalibrationID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,
    MTRClusterThermostatAttributeOccupiedCoolingSetpointID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeOccupiedCoolingSetpointID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterThermostatAttributeOccupiedHeatingSetpointID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeOccupiedHeatingSetpointID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterThermostatAttributeUnoccupiedCoolingSetpointID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedCoolingSetpointID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000013,
    MTRClusterThermostatAttributeUnoccupiedHeatingSetpointID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedHeatingSetpointID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000014,
    MTRClusterThermostatAttributeMinHeatSetpointLimitID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeMinHeatSetpointLimitID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000015,
    MTRClusterThermostatAttributeMaxHeatSetpointLimitID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeMaxHeatSetpointLimitID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000016,
    MTRClusterThermostatAttributeMinCoolSetpointLimitID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeMinCoolSetpointLimitID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000017,
    MTRClusterThermostatAttributeMaxCoolSetpointLimitID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeMaxCoolSetpointLimitID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000018,
    MTRClusterThermostatAttributeMinSetpointDeadBandID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeMinSetpointDeadBandID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000019,
    MTRClusterThermostatAttributeRemoteSensingID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeRemoteSensingID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001A,
    MTRClusterThermostatAttributeControlSequenceOfOperationID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeControlSequenceOfOperationID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001B,
    MTRClusterThermostatAttributeSystemModeID API_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeSystemModeID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001C,
    MTRClusterThermostatAttributeThermostatRunningModeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeThermostatRunningModeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001E,
    MTRClusterThermostatAttributeStartOfWeekID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeStartOfWeekID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000020,
    MTRClusterThermostatAttributeNumberOfWeeklyTransitionsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeNumberOfWeeklyTransitionsID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000021,
    MTRClusterThermostatAttributeNumberOfDailyTransitionsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeNumberOfDailyTransitionsID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000022,
    MTRClusterThermostatAttributeTemperatureSetpointHoldID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeTemperatureSetpointHoldID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000023,
    MTRClusterThermostatAttributeTemperatureSetpointHoldDurationID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeTemperatureSetpointHoldDurationID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000024,
    MTRClusterThermostatAttributeThermostatProgrammingOperationModeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeThermostatProgrammingOperationModeID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000025,
    MTRClusterThermostatAttributeThermostatRunningStateID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeThermostatRunningStateID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000029,
    MTRClusterThermostatAttributeSetpointChangeSourceID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeSetpointChangeSourceID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000030,
    MTRClusterThermostatAttributeSetpointChangeAmountID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeSetpointChangeAmountID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000031,
    MTRClusterThermostatAttributeSetpointChangeSourceTimestampID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeSetpointChangeSourceTimestampID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000032,
    MTRClusterThermostatAttributeOccupiedSetbackID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeOccupiedSetbackID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000034,
    MTRClusterThermostatAttributeOccupiedSetbackMinID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeOccupiedSetbackMinID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000035,
    MTRClusterThermostatAttributeOccupiedSetbackMaxID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeOccupiedSetbackMaxID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000036,
    MTRClusterThermostatAttributeUnoccupiedSetbackID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedSetbackID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000037,
    MTRClusterThermostatAttributeUnoccupiedSetbackMinID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedSetbackMinID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000038,
    MTRClusterThermostatAttributeUnoccupiedSetbackMaxID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedSetbackMaxID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000039,
    MTRClusterThermostatAttributeEmergencyHeatDeltaID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeEmergencyHeatDeltaID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000003A,
    MTRClusterThermostatAttributeACTypeID API_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeACTypeID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000040,
    MTRClusterThermostatAttributeACCapacityID API_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeACCapacityID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000041,
    MTRClusterThermostatAttributeACRefrigerantTypeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeACRefrigerantTypeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000042,
    MTRClusterThermostatAttributeACCompressorTypeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeACCompressorTypeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000043,
    MTRClusterThermostatAttributeACErrorCodeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeACErrorCodeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000044,
    MTRClusterThermostatAttributeACLouverPositionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeACLouverPositionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000045,
    MTRClusterThermostatAttributeACCoilTemperatureID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeACCoilTemperatureID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000046,
    MTRClusterThermostatAttributeACCapacityformatID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeACCapacityformatID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000047,
    MTRClusterThermostatAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterThermostatAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterThermostatAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterThermostatAttributeFeatureMapID API_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeFeatureMapID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterThermostatAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster Thermostat attributes
    MTRAttributeIDTypeClusterThermostatAttributeLocalTemperatureID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterThermostatAttributeOutdoorTemperatureID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterThermostatAttributeOccupancyID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterThermostatAttributeAbsMinHeatSetpointLimitID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterThermostatAttributeAbsMaxHeatSetpointLimitID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRAttributeIDTypeClusterThermostatAttributeAbsMinCoolSetpointLimitID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTRAttributeIDTypeClusterThermostatAttributeAbsMaxCoolSetpointLimitID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,
    MTRAttributeIDTypeClusterThermostatAttributePICoolingDemandID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000007,
    MTRAttributeIDTypeClusterThermostatAttributePIHeatingDemandID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000008,
    MTRAttributeIDTypeClusterThermostatAttributeHVACSystemTypeConfigurationID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000009,
    MTRAttributeIDTypeClusterThermostatAttributeLocalTemperatureCalibrationID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000010,
    MTRAttributeIDTypeClusterThermostatAttributeOccupiedCoolingSetpointID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000011,
    MTRAttributeIDTypeClusterThermostatAttributeOccupiedHeatingSetpointID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000012,
    MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedCoolingSetpointID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000013,
    MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedHeatingSetpointID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000014,
    MTRAttributeIDTypeClusterThermostatAttributeMinHeatSetpointLimitID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000015,
    MTRAttributeIDTypeClusterThermostatAttributeMaxHeatSetpointLimitID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000016,
    MTRAttributeIDTypeClusterThermostatAttributeMinCoolSetpointLimitID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000017,
    MTRAttributeIDTypeClusterThermostatAttributeMaxCoolSetpointLimitID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000018,
    MTRAttributeIDTypeClusterThermostatAttributeMinSetpointDeadBandID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000019,
    MTRAttributeIDTypeClusterThermostatAttributeRemoteSensingID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000001A,
    MTRAttributeIDTypeClusterThermostatAttributeControlSequenceOfOperationID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000001B,
    MTRAttributeIDTypeClusterThermostatAttributeSystemModeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000001C,
    MTRAttributeIDTypeClusterThermostatAttributeThermostatRunningModeID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000001E,
    MTRAttributeIDTypeClusterThermostatAttributeStartOfWeekID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000020,
    MTRAttributeIDTypeClusterThermostatAttributeNumberOfWeeklyTransitionsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000021,
    MTRAttributeIDTypeClusterThermostatAttributeNumberOfDailyTransitionsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000022,
    MTRAttributeIDTypeClusterThermostatAttributeTemperatureSetpointHoldID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000023,
    MTRAttributeIDTypeClusterThermostatAttributeTemperatureSetpointHoldDurationID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000024,
    MTRAttributeIDTypeClusterThermostatAttributeThermostatProgrammingOperationModeID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000025,
    MTRAttributeIDTypeClusterThermostatAttributeThermostatRunningStateID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000029,
    MTRAttributeIDTypeClusterThermostatAttributeSetpointChangeSourceID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000030,
    MTRAttributeIDTypeClusterThermostatAttributeSetpointChangeAmountID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000031,
    MTRAttributeIDTypeClusterThermostatAttributeSetpointChangeSourceTimestampID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000032,
    MTRAttributeIDTypeClusterThermostatAttributeOccupiedSetbackID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000034,
    MTRAttributeIDTypeClusterThermostatAttributeOccupiedSetbackMinID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000035,
    MTRAttributeIDTypeClusterThermostatAttributeOccupiedSetbackMaxID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000036,
    MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedSetbackID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000037,
    MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedSetbackMinID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000038,
    MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedSetbackMaxID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000039,
    MTRAttributeIDTypeClusterThermostatAttributeEmergencyHeatDeltaID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000003A,
    MTRAttributeIDTypeClusterThermostatAttributeACTypeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000040,
    MTRAttributeIDTypeClusterThermostatAttributeACCapacityID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000041,
    MTRAttributeIDTypeClusterThermostatAttributeACRefrigerantTypeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000042,
    MTRAttributeIDTypeClusterThermostatAttributeACCompressorTypeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000043,
    MTRAttributeIDTypeClusterThermostatAttributeACErrorCodeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000044,
    MTRAttributeIDTypeClusterThermostatAttributeACLouverPositionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000045,
    MTRAttributeIDTypeClusterThermostatAttributeACCoilTemperatureID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000046,
    MTRAttributeIDTypeClusterThermostatAttributeACCapacityformatID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000047,
    MTRAttributeIDTypeClusterThermostatAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterThermostatAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterThermostatAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterThermostatAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterThermostatAttributeClusterRevisionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster FanControl deprecated attribute names
    MTRClusterFanControlAttributeFanModeID API_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeFanModeID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterFanControlAttributeFanModeSequenceID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFanControlAttributeFanModeSequenceID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterFanControlAttributePercentSettingID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFanControlAttributePercentSettingID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterFanControlAttributePercentCurrentID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFanControlAttributePercentCurrentID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterFanControlAttributeSpeedMaxID API_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeSpeedMaxID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterFanControlAttributeSpeedSettingID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFanControlAttributeSpeedSettingID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterFanControlAttributeSpeedCurrentID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFanControlAttributeSpeedCurrentID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterFanControlAttributeRockSupportID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFanControlAttributeRockSupportID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterFanControlAttributeRockSettingID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFanControlAttributeRockSettingID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterFanControlAttributeWindSupportID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFanControlAttributeWindSupportID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterFanControlAttributeWindSettingID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFanControlAttributeWindSettingID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterFanControlAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFanControlAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterFanControlAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFanControlAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterFanControlAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFanControlAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterFanControlAttributeFeatureMapID API_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeFeatureMapID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterFanControlAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFanControlAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster FanControl attributes
    MTRAttributeIDTypeClusterFanControlAttributeFanModeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterFanControlAttributeFanModeSequenceID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterFanControlAttributePercentSettingID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterFanControlAttributePercentCurrentID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterFanControlAttributeSpeedMaxID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRAttributeIDTypeClusterFanControlAttributeSpeedSettingID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTRAttributeIDTypeClusterFanControlAttributeSpeedCurrentID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,
    MTRAttributeIDTypeClusterFanControlAttributeRockSupportID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000007,
    MTRAttributeIDTypeClusterFanControlAttributeRockSettingID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000008,
    MTRAttributeIDTypeClusterFanControlAttributeWindSupportID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000009,
    MTRAttributeIDTypeClusterFanControlAttributeWindSettingID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000A,
    MTRAttributeIDTypeClusterFanControlAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterFanControlAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterFanControlAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterFanControlAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterFanControlAttributeClusterRevisionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ThermostatUserInterfaceConfiguration deprecated attribute names
    MTRClusterThermostatUserInterfaceConfigurationAttributeTemperatureDisplayModeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeTemperatureDisplayModeID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterThermostatUserInterfaceConfigurationAttributeKeypadLockoutID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeKeypadLockoutID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterThermostatUserInterfaceConfigurationAttributeScheduleProgrammingVisibilityID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeScheduleProgrammingVisibilityID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterThermostatUserInterfaceConfigurationAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeGeneratedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterThermostatUserInterfaceConfigurationAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeAcceptedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterThermostatUserInterfaceConfigurationAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeAttributeListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterThermostatUserInterfaceConfigurationAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeFeatureMapID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterThermostatUserInterfaceConfigurationAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeClusterRevisionID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ThermostatUserInterfaceConfiguration attributes
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeTemperatureDisplayModeID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeKeypadLockoutID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeScheduleProgrammingVisibilityID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeFeatureMapID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ColorControl deprecated attribute names
    MTRClusterColorControlAttributeCurrentHueID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeCurrentHueID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterColorControlAttributeCurrentSaturationID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeCurrentSaturationID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterColorControlAttributeRemainingTimeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeRemainingTimeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterColorControlAttributeCurrentXID API_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeCurrentXID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterColorControlAttributeCurrentYID API_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeCurrentYID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterColorControlAttributeDriftCompensationID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeDriftCompensationID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterColorControlAttributeCompensationTextID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeCompensationTextID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterColorControlAttributeColorTemperatureMiredsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorTemperatureMiredsID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterColorControlAttributeColorModeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorModeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterColorControlAttributeOptionsID API_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeOptionsID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000F,
    MTRClusterColorControlAttributeNumberOfPrimariesID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeNumberOfPrimariesID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,
    MTRClusterColorControlAttributePrimary1XID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary1XID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterColorControlAttributePrimary1YID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary1YID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterColorControlAttributePrimary1IntensityID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary1IntensityID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000013,
    MTRClusterColorControlAttributePrimary2XID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary2XID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000015,
    MTRClusterColorControlAttributePrimary2YID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary2YID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000016,
    MTRClusterColorControlAttributePrimary2IntensityID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary2IntensityID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000017,
    MTRClusterColorControlAttributePrimary3XID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary3XID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000019,
    MTRClusterColorControlAttributePrimary3YID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary3YID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001A,
    MTRClusterColorControlAttributePrimary3IntensityID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary3IntensityID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001B,
    MTRClusterColorControlAttributePrimary4XID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary4XID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000020,
    MTRClusterColorControlAttributePrimary4YID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary4YID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000021,
    MTRClusterColorControlAttributePrimary4IntensityID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary4IntensityID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000022,
    MTRClusterColorControlAttributePrimary5XID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary5XID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000024,
    MTRClusterColorControlAttributePrimary5YID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary5YID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000025,
    MTRClusterColorControlAttributePrimary5IntensityID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary5IntensityID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000026,
    MTRClusterColorControlAttributePrimary6XID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary6XID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000028,
    MTRClusterColorControlAttributePrimary6YID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary6YID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000029,
    MTRClusterColorControlAttributePrimary6IntensityID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary6IntensityID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002A,
    MTRClusterColorControlAttributeWhitePointXID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeWhitePointXID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000030,
    MTRClusterColorControlAttributeWhitePointYID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeWhitePointYID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000031,
    MTRClusterColorControlAttributeColorPointRXID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointRXID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000032,
    MTRClusterColorControlAttributeColorPointRYID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointRYID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000033,
    MTRClusterColorControlAttributeColorPointRIntensityID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointRIntensityID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000034,
    MTRClusterColorControlAttributeColorPointGXID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointGXID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000036,
    MTRClusterColorControlAttributeColorPointGYID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointGYID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000037,
    MTRClusterColorControlAttributeColorPointGIntensityID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointGIntensityID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000038,
    MTRClusterColorControlAttributeColorPointBXID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointBXID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000003A,
    MTRClusterColorControlAttributeColorPointBYID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointBYID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000003B,
    MTRClusterColorControlAttributeColorPointBIntensityID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointBIntensityID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000003C,
    MTRClusterColorControlAttributeEnhancedCurrentHueID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeEnhancedCurrentHueID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004000,
    MTRClusterColorControlAttributeEnhancedColorModeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeEnhancedColorModeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004001,
    MTRClusterColorControlAttributeColorLoopActiveID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorLoopActiveID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004002,
    MTRClusterColorControlAttributeColorLoopDirectionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorLoopDirectionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004003,
    MTRClusterColorControlAttributeColorLoopTimeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorLoopTimeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004004,
    MTRClusterColorControlAttributeColorLoopStartEnhancedHueID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorLoopStartEnhancedHueID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004005,
    MTRClusterColorControlAttributeColorLoopStoredEnhancedHueID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorLoopStoredEnhancedHueID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004006,
    MTRClusterColorControlAttributeColorCapabilitiesID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorCapabilitiesID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000400A,
    MTRClusterColorControlAttributeColorTempPhysicalMinMiredsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorTempPhysicalMinMiredsID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000400B,
    MTRClusterColorControlAttributeColorTempPhysicalMaxMiredsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorTempPhysicalMaxMiredsID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000400C,
    MTRClusterColorControlAttributeCoupleColorTempToLevelMinMiredsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeCoupleColorTempToLevelMinMiredsID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000400D,
    MTRClusterColorControlAttributeStartUpColorTemperatureMiredsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeStartUpColorTemperatureMiredsID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004010,
    MTRClusterColorControlAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterColorControlAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterColorControlAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterColorControlAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterColorControlAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ColorControl attributes
    MTRAttributeIDTypeClusterColorControlAttributeCurrentHueID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterColorControlAttributeCurrentSaturationID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterColorControlAttributeRemainingTimeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterColorControlAttributeCurrentXID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterColorControlAttributeCurrentYID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRAttributeIDTypeClusterColorControlAttributeDriftCompensationID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTRAttributeIDTypeClusterColorControlAttributeCompensationTextID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,
    MTRAttributeIDTypeClusterColorControlAttributeColorTemperatureMiredsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000007,
    MTRAttributeIDTypeClusterColorControlAttributeColorModeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000008,
    MTRAttributeIDTypeClusterColorControlAttributeOptionsID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000F,
    MTRAttributeIDTypeClusterColorControlAttributeNumberOfPrimariesID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000010,
    MTRAttributeIDTypeClusterColorControlAttributePrimary1XID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000011,
    MTRAttributeIDTypeClusterColorControlAttributePrimary1YID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000012,
    MTRAttributeIDTypeClusterColorControlAttributePrimary1IntensityID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000013,
    MTRAttributeIDTypeClusterColorControlAttributePrimary2XID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000015,
    MTRAttributeIDTypeClusterColorControlAttributePrimary2YID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000016,
    MTRAttributeIDTypeClusterColorControlAttributePrimary2IntensityID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000017,
    MTRAttributeIDTypeClusterColorControlAttributePrimary3XID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000019,
    MTRAttributeIDTypeClusterColorControlAttributePrimary3YID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000001A,
    MTRAttributeIDTypeClusterColorControlAttributePrimary3IntensityID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000001B,
    MTRAttributeIDTypeClusterColorControlAttributePrimary4XID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000020,
    MTRAttributeIDTypeClusterColorControlAttributePrimary4YID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000021,
    MTRAttributeIDTypeClusterColorControlAttributePrimary4IntensityID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000022,
    MTRAttributeIDTypeClusterColorControlAttributePrimary5XID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000024,
    MTRAttributeIDTypeClusterColorControlAttributePrimary5YID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000025,
    MTRAttributeIDTypeClusterColorControlAttributePrimary5IntensityID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000026,
    MTRAttributeIDTypeClusterColorControlAttributePrimary6XID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000028,
    MTRAttributeIDTypeClusterColorControlAttributePrimary6YID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000029,
    MTRAttributeIDTypeClusterColorControlAttributePrimary6IntensityID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000002A,
    MTRAttributeIDTypeClusterColorControlAttributeWhitePointXID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000030,
    MTRAttributeIDTypeClusterColorControlAttributeWhitePointYID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000031,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointRXID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000032,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointRYID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000033,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointRIntensityID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000034,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointGXID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000036,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointGYID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000037,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointGIntensityID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000038,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointBXID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000003A,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointBYID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000003B,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointBIntensityID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000003C,
    MTRAttributeIDTypeClusterColorControlAttributeEnhancedCurrentHueID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004000,
    MTRAttributeIDTypeClusterColorControlAttributeEnhancedColorModeID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004001,
    MTRAttributeIDTypeClusterColorControlAttributeColorLoopActiveID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004002,
    MTRAttributeIDTypeClusterColorControlAttributeColorLoopDirectionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004003,
    MTRAttributeIDTypeClusterColorControlAttributeColorLoopTimeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004004,
    MTRAttributeIDTypeClusterColorControlAttributeColorLoopStartEnhancedHueID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004005,
    MTRAttributeIDTypeClusterColorControlAttributeColorLoopStoredEnhancedHueID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004006,
    MTRAttributeIDTypeClusterColorControlAttributeColorCapabilitiesID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000400A,
    MTRAttributeIDTypeClusterColorControlAttributeColorTempPhysicalMinMiredsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000400B,
    MTRAttributeIDTypeClusterColorControlAttributeColorTempPhysicalMaxMiredsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000400C,
    MTRAttributeIDTypeClusterColorControlAttributeCoupleColorTempToLevelMinMiredsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000400D,
    MTRAttributeIDTypeClusterColorControlAttributeStartUpColorTemperatureMiredsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004010,
    MTRAttributeIDTypeClusterColorControlAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterColorControlAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterColorControlAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterColorControlAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterColorControlAttributeClusterRevisionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster BallastConfiguration deprecated attribute names
    MTRClusterBallastConfigurationAttributePhysicalMinLevelID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributePhysicalMinLevelID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterBallastConfigurationAttributePhysicalMaxLevelID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributePhysicalMaxLevelID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterBallastConfigurationAttributeBallastStatusID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeBallastStatusID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterBallastConfigurationAttributeMinLevelID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeMinLevelID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,
    MTRClusterBallastConfigurationAttributeMaxLevelID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeMaxLevelID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterBallastConfigurationAttributeIntrinsicBalanceFactorID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeIntrinsicBallastFactorID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000014,
    MTRClusterBallastConfigurationAttributeBallastFactorAdjustmentID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeBallastFactorAdjustmentID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000015,
    MTRClusterBallastConfigurationAttributeLampQuantityID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeLampQuantityID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000020,
    MTRClusterBallastConfigurationAttributeLampTypeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeLampTypeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000030,
    MTRClusterBallastConfigurationAttributeLampManufacturerID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeLampManufacturerID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000031,
    MTRClusterBallastConfigurationAttributeLampRatedHoursID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeLampRatedHoursID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000032,
    MTRClusterBallastConfigurationAttributeLampBurnHoursID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeLampBurnHoursID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000033,
    MTRClusterBallastConfigurationAttributeLampAlarmModeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeLampAlarmModeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000034,
    MTRClusterBallastConfigurationAttributeLampBurnHoursTripPointID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeLampBurnHoursTripPointID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000035,
    MTRClusterBallastConfigurationAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeGeneratedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterBallastConfigurationAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeAcceptedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterBallastConfigurationAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterBallastConfigurationAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterBallastConfigurationAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster BallastConfiguration attributes
    MTRAttributeIDTypeClusterBallastConfigurationAttributePhysicalMinLevelID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterBallastConfigurationAttributePhysicalMaxLevelID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeBallastStatusID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeMinLevelID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000010,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeMaxLevelID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000011,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeIntrinsicBallastFactorID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000014,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeBallastFactorAdjustmentID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000015,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeLampQuantityID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000020,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeLampTypeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000030,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeLampManufacturerID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000031,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeLampRatedHoursID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000032,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeLampBurnHoursID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000033,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeLampAlarmModeID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000034,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeLampBurnHoursTripPointID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000035,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeFeatureMapID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster IlluminanceMeasurement deprecated attribute names
    MTRClusterIlluminanceMeasurementAttributeMeasuredValueID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeMeasuredValueID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterIlluminanceMeasurementAttributeMinMeasuredValueID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeMinMeasuredValueID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterIlluminanceMeasurementAttributeMaxMeasuredValueID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeMaxMeasuredValueID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterIlluminanceMeasurementAttributeToleranceID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeToleranceID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterIlluminanceMeasurementAttributeLightSensorTypeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeLightSensorTypeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterIlluminanceMeasurementAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeGeneratedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterIlluminanceMeasurementAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeAcceptedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterIlluminanceMeasurementAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterIlluminanceMeasurementAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterIlluminanceMeasurementAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster IlluminanceMeasurement attributes
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeMeasuredValueID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeMinMeasuredValueID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeMaxMeasuredValueID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeToleranceID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeLightSensorTypeID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeFeatureMapID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster TemperatureMeasurement deprecated attribute names
    MTRClusterTemperatureMeasurementAttributeMeasuredValueID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeMeasuredValueID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterTemperatureMeasurementAttributeMinMeasuredValueID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeMinMeasuredValueID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterTemperatureMeasurementAttributeMaxMeasuredValueID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeMaxMeasuredValueID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterTemperatureMeasurementAttributeToleranceID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeToleranceID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterTemperatureMeasurementAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeGeneratedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterTemperatureMeasurementAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeAcceptedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterTemperatureMeasurementAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterTemperatureMeasurementAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterTemperatureMeasurementAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster TemperatureMeasurement attributes
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeMeasuredValueID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeMinMeasuredValueID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeMaxMeasuredValueID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeToleranceID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeFeatureMapID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster PressureMeasurement deprecated attribute names
    MTRClusterPressureMeasurementAttributeMeasuredValueID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeMeasuredValueID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterPressureMeasurementAttributeMinMeasuredValueID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeMinMeasuredValueID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterPressureMeasurementAttributeMaxMeasuredValueID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeMaxMeasuredValueID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterPressureMeasurementAttributeToleranceID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeToleranceID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterPressureMeasurementAttributeScaledValueID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeScaledValueID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,
    MTRClusterPressureMeasurementAttributeMinScaledValueID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeMinScaledValueID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterPressureMeasurementAttributeMaxScaledValueID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeMaxScaledValueID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterPressureMeasurementAttributeScaledToleranceID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeScaledToleranceID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000013,
    MTRClusterPressureMeasurementAttributeScaleID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeScaleID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000014,
    MTRClusterPressureMeasurementAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeGeneratedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterPressureMeasurementAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterPressureMeasurementAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterPressureMeasurementAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterPressureMeasurementAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster PressureMeasurement attributes
    MTRAttributeIDTypeClusterPressureMeasurementAttributeMeasuredValueID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeMinMeasuredValueID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeMaxMeasuredValueID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeToleranceID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeScaledValueID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000010,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeMinScaledValueID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000011,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeMaxScaledValueID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000012,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeScaledToleranceID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000013,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeScaleID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000014,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeFeatureMapID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster FlowMeasurement deprecated attribute names
    MTRClusterFlowMeasurementAttributeMeasuredValueID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeMeasuredValueID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterFlowMeasurementAttributeMinMeasuredValueID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeMinMeasuredValueID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterFlowMeasurementAttributeMaxMeasuredValueID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeMaxMeasuredValueID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterFlowMeasurementAttributeToleranceID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeToleranceID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterFlowMeasurementAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterFlowMeasurementAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterFlowMeasurementAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterFlowMeasurementAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterFlowMeasurementAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster FlowMeasurement attributes
    MTRAttributeIDTypeClusterFlowMeasurementAttributeMeasuredValueID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeMinMeasuredValueID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeMaxMeasuredValueID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeToleranceID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster RelativeHumidityMeasurement deprecated attribute names
    MTRClusterRelativeHumidityMeasurementAttributeMeasuredValueID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeMeasuredValueID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterRelativeHumidityMeasurementAttributeMinMeasuredValueID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeMinMeasuredValueID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterRelativeHumidityMeasurementAttributeMaxMeasuredValueID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeMaxMeasuredValueID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterRelativeHumidityMeasurementAttributeToleranceID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeToleranceID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterRelativeHumidityMeasurementAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeGeneratedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterRelativeHumidityMeasurementAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeAcceptedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterRelativeHumidityMeasurementAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeAttributeListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterRelativeHumidityMeasurementAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterRelativeHumidityMeasurementAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeClusterRevisionID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster RelativeHumidityMeasurement attributes
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeMeasuredValueID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeMinMeasuredValueID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeMaxMeasuredValueID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeToleranceID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeFeatureMapID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster OccupancySensing deprecated attribute names
    MTRClusterOccupancySensingAttributeOccupancyID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributeOccupancyID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterOccupancySensingAttributeOccupancySensorTypeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributeOccupancySensorTypeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterOccupancySensingAttributeOccupancySensorTypeBitmapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributeOccupancySensorTypeBitmapID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterOccupancySensingAttributePirOccupiedToUnoccupiedDelayID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributePIROccupiedToUnoccupiedDelayID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,
    MTRClusterOccupancySensingAttributePirUnoccupiedToOccupiedDelayID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributePIRUnoccupiedToOccupiedDelayID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterOccupancySensingAttributePirUnoccupiedToOccupiedThresholdID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributePIRUnoccupiedToOccupiedThresholdID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterOccupancySensingAttributeUltrasonicOccupiedToUnoccupiedDelayID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributeUltrasonicOccupiedToUnoccupiedDelayID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000020,
    MTRClusterOccupancySensingAttributeUltrasonicUnoccupiedToOccupiedDelayID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributeUltrasonicUnoccupiedToOccupiedDelayID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000021,
    MTRClusterOccupancySensingAttributeUltrasonicUnoccupiedToOccupiedThresholdID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributeUltrasonicUnoccupiedToOccupiedThresholdID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000022,
    MTRClusterOccupancySensingAttributePhysicalContactOccupiedToUnoccupiedDelayID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributePhysicalContactOccupiedToUnoccupiedDelayID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000030,
    MTRClusterOccupancySensingAttributePhysicalContactUnoccupiedToOccupiedDelayID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributePhysicalContactUnoccupiedToOccupiedDelayID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000031,
    MTRClusterOccupancySensingAttributePhysicalContactUnoccupiedToOccupiedThresholdID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributePhysicalContactUnoccupiedToOccupiedThresholdID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000032,
    MTRClusterOccupancySensingAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterOccupancySensingAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterOccupancySensingAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterOccupancySensingAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterOccupancySensingAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster OccupancySensing attributes
    MTRAttributeIDTypeClusterOccupancySensingAttributeOccupancyID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterOccupancySensingAttributeOccupancySensorTypeID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterOccupancySensingAttributeOccupancySensorTypeBitmapID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterOccupancySensingAttributePIROccupiedToUnoccupiedDelayID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000010,
    MTRAttributeIDTypeClusterOccupancySensingAttributePIRUnoccupiedToOccupiedDelayID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000011,
    MTRAttributeIDTypeClusterOccupancySensingAttributePIRUnoccupiedToOccupiedThresholdID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000012,
    MTRAttributeIDTypeClusterOccupancySensingAttributeUltrasonicOccupiedToUnoccupiedDelayID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000020,
    MTRAttributeIDTypeClusterOccupancySensingAttributeUltrasonicUnoccupiedToOccupiedDelayID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000021,
    MTRAttributeIDTypeClusterOccupancySensingAttributeUltrasonicUnoccupiedToOccupiedThresholdID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000022,
    MTRAttributeIDTypeClusterOccupancySensingAttributePhysicalContactOccupiedToUnoccupiedDelayID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000030,
    MTRAttributeIDTypeClusterOccupancySensingAttributePhysicalContactUnoccupiedToOccupiedDelayID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000031,
    MTRAttributeIDTypeClusterOccupancySensingAttributePhysicalContactUnoccupiedToOccupiedThresholdID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000032,
    MTRAttributeIDTypeClusterOccupancySensingAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterOccupancySensingAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterOccupancySensingAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterOccupancySensingAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterOccupancySensingAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster WakeOnLan deprecated attribute names
    MTRClusterWakeOnLanAttributeMACAddressID API_DEPRECATED("Please use MTRAttributeIDTypeClusterWakeOnLANAttributeMACAddressID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterWakeOnLanAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWakeOnLANAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterWakeOnLanAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWakeOnLANAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterWakeOnLanAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWakeOnLANAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterWakeOnLanAttributeFeatureMapID API_DEPRECATED("Please use MTRAttributeIDTypeClusterWakeOnLANAttributeFeatureMapID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterWakeOnLanAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWakeOnLANAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster WakeOnLAN attributes
    MTRAttributeIDTypeClusterWakeOnLANAttributeMACAddressID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterWakeOnLANAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterWakeOnLANAttributeAcceptedCommandListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterWakeOnLANAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterWakeOnLANAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterWakeOnLANAttributeClusterRevisionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Channel deprecated attribute names
    MTRClusterChannelAttributeChannelListID API_DEPRECATED("Please use MTRAttributeIDTypeClusterChannelAttributeChannelListID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterChannelAttributeLineupID API_DEPRECATED("Please use MTRAttributeIDTypeClusterChannelAttributeLineupID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterChannelAttributeCurrentChannelID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterChannelAttributeCurrentChannelID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterChannelAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterChannelAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterChannelAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterChannelAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterChannelAttributeAttributeListID API_DEPRECATED("Please use MTRAttributeIDTypeClusterChannelAttributeAttributeListID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterChannelAttributeFeatureMapID API_DEPRECATED("Please use MTRAttributeIDTypeClusterChannelAttributeFeatureMapID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterChannelAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterChannelAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster Channel attributes
    MTRAttributeIDTypeClusterChannelAttributeChannelListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterChannelAttributeLineupID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterChannelAttributeCurrentChannelID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterChannelAttributeGeneratedCommandListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterChannelAttributeAcceptedCommandListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterChannelAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterChannelAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterChannelAttributeClusterRevisionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster TargetNavigator deprecated attribute names
    MTRClusterTargetNavigatorAttributeTargetListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTargetNavigatorAttributeTargetListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterTargetNavigatorAttributeCurrentTargetID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTargetNavigatorAttributeCurrentTargetID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterTargetNavigatorAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTargetNavigatorAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterTargetNavigatorAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTargetNavigatorAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterTargetNavigatorAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTargetNavigatorAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterTargetNavigatorAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTargetNavigatorAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterTargetNavigatorAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTargetNavigatorAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster TargetNavigator attributes
    MTRAttributeIDTypeClusterTargetNavigatorAttributeTargetListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterTargetNavigatorAttributeCurrentTargetID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterTargetNavigatorAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterTargetNavigatorAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterTargetNavigatorAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterTargetNavigatorAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterTargetNavigatorAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster MediaPlayback deprecated attribute names
    MTRClusterMediaPlaybackAttributeCurrentStateID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeCurrentStateID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterMediaPlaybackAttributeStartTimeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeStartTimeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterMediaPlaybackAttributeDurationID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeDurationID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterMediaPlaybackAttributeSampledPositionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeSampledPositionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterMediaPlaybackAttributePlaybackSpeedID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaPlaybackAttributePlaybackSpeedID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterMediaPlaybackAttributeSeekRangeEndID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeSeekRangeEndID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterMediaPlaybackAttributeSeekRangeStartID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeSeekRangeStartID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterMediaPlaybackAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterMediaPlaybackAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterMediaPlaybackAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterMediaPlaybackAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterMediaPlaybackAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster MediaPlayback attributes
    MTRAttributeIDTypeClusterMediaPlaybackAttributeCurrentStateID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeStartTimeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeDurationID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeSampledPositionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterMediaPlaybackAttributePlaybackSpeedID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeSeekRangeEndID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeSeekRangeStartID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeClusterRevisionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster MediaInput deprecated attribute names
    MTRClusterMediaInputAttributeInputListID API_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaInputAttributeInputListID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterMediaInputAttributeCurrentInputID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaInputAttributeCurrentInputID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterMediaInputAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaInputAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterMediaInputAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaInputAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterMediaInputAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaInputAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterMediaInputAttributeFeatureMapID API_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaInputAttributeFeatureMapID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterMediaInputAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaInputAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster MediaInput attributes
    MTRAttributeIDTypeClusterMediaInputAttributeInputListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterMediaInputAttributeCurrentInputID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterMediaInputAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterMediaInputAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterMediaInputAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterMediaInputAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterMediaInputAttributeClusterRevisionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster LowPower deprecated attribute names
    MTRClusterLowPowerAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLowPowerAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterLowPowerAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLowPowerAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterLowPowerAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLowPowerAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterLowPowerAttributeFeatureMapID API_DEPRECATED("Please use MTRAttributeIDTypeClusterLowPowerAttributeFeatureMapID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterLowPowerAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLowPowerAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster LowPower attributes
    MTRAttributeIDTypeClusterLowPowerAttributeGeneratedCommandListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterLowPowerAttributeAcceptedCommandListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterLowPowerAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterLowPowerAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterLowPowerAttributeClusterRevisionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster KeypadInput deprecated attribute names
    MTRClusterKeypadInputAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterKeypadInputAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterKeypadInputAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterKeypadInputAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterKeypadInputAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterKeypadInputAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterKeypadInputAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterKeypadInputAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterKeypadInputAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterKeypadInputAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster KeypadInput attributes
    MTRAttributeIDTypeClusterKeypadInputAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterKeypadInputAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterKeypadInputAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterKeypadInputAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterKeypadInputAttributeClusterRevisionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ContentLauncher deprecated attribute names
    MTRClusterContentLauncherAttributeAcceptHeaderID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterContentLauncherAttributeAcceptHeaderID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterContentLauncherAttributeSupportedStreamingProtocolsID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterContentLauncherAttributeSupportedStreamingProtocolsID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterContentLauncherAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterContentLauncherAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterContentLauncherAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterContentLauncherAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterContentLauncherAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterContentLauncherAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterContentLauncherAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterContentLauncherAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterContentLauncherAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterContentLauncherAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ContentLauncher attributes
    MTRAttributeIDTypeClusterContentLauncherAttributeAcceptHeaderID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterContentLauncherAttributeSupportedStreamingProtocolsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterContentLauncherAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterContentLauncherAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterContentLauncherAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterContentLauncherAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterContentLauncherAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster AudioOutput deprecated attribute names
    MTRClusterAudioOutputAttributeOutputListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAudioOutputAttributeOutputListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterAudioOutputAttributeCurrentOutputID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAudioOutputAttributeCurrentOutputID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterAudioOutputAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAudioOutputAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterAudioOutputAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAudioOutputAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterAudioOutputAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAudioOutputAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterAudioOutputAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAudioOutputAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterAudioOutputAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAudioOutputAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster AudioOutput attributes
    MTRAttributeIDTypeClusterAudioOutputAttributeOutputListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterAudioOutputAttributeCurrentOutputID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterAudioOutputAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterAudioOutputAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterAudioOutputAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterAudioOutputAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterAudioOutputAttributeClusterRevisionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ApplicationLauncher deprecated attribute names
    MTRClusterApplicationLauncherAttributeCatalogListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationLauncherAttributeCatalogListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterApplicationLauncherAttributeCurrentAppID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationLauncherAttributeCurrentAppID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterApplicationLauncherAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationLauncherAttributeGeneratedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterApplicationLauncherAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationLauncherAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterApplicationLauncherAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationLauncherAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterApplicationLauncherAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationLauncherAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterApplicationLauncherAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationLauncherAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ApplicationLauncher attributes
    MTRAttributeIDTypeClusterApplicationLauncherAttributeCatalogListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterApplicationLauncherAttributeCurrentAppID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterApplicationLauncherAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterApplicationLauncherAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterApplicationLauncherAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterApplicationLauncherAttributeFeatureMapID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterApplicationLauncherAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ApplicationBasic deprecated attribute names
    MTRClusterApplicationBasicAttributeVendorNameID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationBasicAttributeVendorNameID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterApplicationBasicAttributeVendorIDID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationBasicAttributeVendorIDID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterApplicationBasicAttributeApplicationNameID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationBasicAttributeApplicationNameID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterApplicationBasicAttributeProductIDID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationBasicAttributeProductIDID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterApplicationBasicAttributeApplicationID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationBasicAttributeApplicationID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterApplicationBasicAttributeStatusID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationBasicAttributeStatusID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterApplicationBasicAttributeApplicationVersionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationBasicAttributeApplicationVersionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterApplicationBasicAttributeAllowedVendorListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationBasicAttributeAllowedVendorListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterApplicationBasicAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationBasicAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterApplicationBasicAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationBasicAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterApplicationBasicAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationBasicAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterApplicationBasicAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationBasicAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterApplicationBasicAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationBasicAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ApplicationBasic attributes
    MTRAttributeIDTypeClusterApplicationBasicAttributeVendorNameID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterApplicationBasicAttributeVendorIDID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterApplicationBasicAttributeApplicationNameID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterApplicationBasicAttributeProductIDID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterApplicationBasicAttributeApplicationID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRAttributeIDTypeClusterApplicationBasicAttributeStatusID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTRAttributeIDTypeClusterApplicationBasicAttributeApplicationVersionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,
    MTRAttributeIDTypeClusterApplicationBasicAttributeAllowedVendorListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000007,
    MTRAttributeIDTypeClusterApplicationBasicAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterApplicationBasicAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterApplicationBasicAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterApplicationBasicAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterApplicationBasicAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster AccountLogin deprecated attribute names
    MTRClusterAccountLoginAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAccountLoginAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterAccountLoginAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAccountLoginAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterAccountLoginAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAccountLoginAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterAccountLoginAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAccountLoginAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterAccountLoginAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAccountLoginAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster AccountLogin attributes
    MTRAttributeIDTypeClusterAccountLoginAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterAccountLoginAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterAccountLoginAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterAccountLoginAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterAccountLoginAttributeClusterRevisionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ElectricalMeasurement deprecated attribute names
    MTRClusterElectricalMeasurementAttributeMeasurementTypeID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasurementTypeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterElectricalMeasurementAttributeDcVoltageID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000100,
    MTRClusterElectricalMeasurementAttributeDcVoltageMinID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageMinID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000101,
    MTRClusterElectricalMeasurementAttributeDcVoltageMaxID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageMaxID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000102,
    MTRClusterElectricalMeasurementAttributeDcCurrentID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000103,
    MTRClusterElectricalMeasurementAttributeDcCurrentMinID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentMinID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000104,
    MTRClusterElectricalMeasurementAttributeDcCurrentMaxID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentMaxID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000105,
    MTRClusterElectricalMeasurementAttributeDcPowerID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000106,
    MTRClusterElectricalMeasurementAttributeDcPowerMinID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerMinID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000107,
    MTRClusterElectricalMeasurementAttributeDcPowerMaxID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerMaxID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000108,
    MTRClusterElectricalMeasurementAttributeDcVoltageMultiplierID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageMultiplierID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000200,
    MTRClusterElectricalMeasurementAttributeDcVoltageDivisorID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageDivisorID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000201,
    MTRClusterElectricalMeasurementAttributeDcCurrentMultiplierID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentMultiplierID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000202,
    MTRClusterElectricalMeasurementAttributeDcCurrentDivisorID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentDivisorID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000203,
    MTRClusterElectricalMeasurementAttributeDcPowerMultiplierID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerMultiplierID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000204,
    MTRClusterElectricalMeasurementAttributeDcPowerDivisorID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerDivisorID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000205,
    MTRClusterElectricalMeasurementAttributeAcFrequencyID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000300,
    MTRClusterElectricalMeasurementAttributeAcFrequencyMinID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyMinID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000301,
    MTRClusterElectricalMeasurementAttributeAcFrequencyMaxID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyMaxID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000302,
    MTRClusterElectricalMeasurementAttributeNeutralCurrentID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeNeutralCurrentID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000303,
    MTRClusterElectricalMeasurementAttributeTotalActivePowerID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeTotalActivePowerID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000304,
    MTRClusterElectricalMeasurementAttributeTotalReactivePowerID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeTotalReactivePowerID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000305,
    MTRClusterElectricalMeasurementAttributeTotalApparentPowerID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeTotalApparentPowerID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000306,
    MTRClusterElectricalMeasurementAttributeMeasured1stHarmonicCurrentID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured1stHarmonicCurrentID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000307,
    MTRClusterElectricalMeasurementAttributeMeasured3rdHarmonicCurrentID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured3rdHarmonicCurrentID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000308,
    MTRClusterElectricalMeasurementAttributeMeasured5thHarmonicCurrentID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured5thHarmonicCurrentID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000309,
    MTRClusterElectricalMeasurementAttributeMeasured7thHarmonicCurrentID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured7thHarmonicCurrentID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000030A,
    MTRClusterElectricalMeasurementAttributeMeasured9thHarmonicCurrentID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured9thHarmonicCurrentID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000030B,
    MTRClusterElectricalMeasurementAttributeMeasured11thHarmonicCurrentID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured11thHarmonicCurrentID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000030C,
    MTRClusterElectricalMeasurementAttributeMeasuredPhase1stHarmonicCurrentID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase1stHarmonicCurrentID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000030D,
    MTRClusterElectricalMeasurementAttributeMeasuredPhase3rdHarmonicCurrentID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase3rdHarmonicCurrentID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000030E,
    MTRClusterElectricalMeasurementAttributeMeasuredPhase5thHarmonicCurrentID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase5thHarmonicCurrentID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000030F,
    MTRClusterElectricalMeasurementAttributeMeasuredPhase7thHarmonicCurrentID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase7thHarmonicCurrentID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000310,
    MTRClusterElectricalMeasurementAttributeMeasuredPhase9thHarmonicCurrentID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase9thHarmonicCurrentID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000311,
    MTRClusterElectricalMeasurementAttributeMeasuredPhase11thHarmonicCurrentID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase11thHarmonicCurrentID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000312,
    MTRClusterElectricalMeasurementAttributeAcFrequencyMultiplierID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyMultiplierID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000400,
    MTRClusterElectricalMeasurementAttributeAcFrequencyDivisorID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyDivisorID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000401,
    MTRClusterElectricalMeasurementAttributePowerMultiplierID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerMultiplierID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000402,
    MTRClusterElectricalMeasurementAttributePowerDivisorID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerDivisorID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000403,
    MTRClusterElectricalMeasurementAttributeHarmonicCurrentMultiplierID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeHarmonicCurrentMultiplierID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000404,
    MTRClusterElectricalMeasurementAttributePhaseHarmonicCurrentMultiplierID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributePhaseHarmonicCurrentMultiplierID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000405,
    MTRClusterElectricalMeasurementAttributeInstantaneousVoltageID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousVoltageID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000500,
    MTRClusterElectricalMeasurementAttributeInstantaneousLineCurrentID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousLineCurrentID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000501,
    MTRClusterElectricalMeasurementAttributeInstantaneousActiveCurrentID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousActiveCurrentID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000502,
    MTRClusterElectricalMeasurementAttributeInstantaneousReactiveCurrentID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousReactiveCurrentID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000503,
    MTRClusterElectricalMeasurementAttributeInstantaneousPowerID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousPowerID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000504,
    MTRClusterElectricalMeasurementAttributeRmsVoltageID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000505,
    MTRClusterElectricalMeasurementAttributeRmsVoltageMinID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMinID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000506,
    MTRClusterElectricalMeasurementAttributeRmsVoltageMaxID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMaxID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000507,
    MTRClusterElectricalMeasurementAttributeRmsCurrentID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000508,
    MTRClusterElectricalMeasurementAttributeRmsCurrentMinID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMinID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000509,
    MTRClusterElectricalMeasurementAttributeRmsCurrentMaxID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMaxID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000050A,
    MTRClusterElectricalMeasurementAttributeActivePowerID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000050B,
    MTRClusterElectricalMeasurementAttributeActivePowerMinID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMinID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000050C,
    MTRClusterElectricalMeasurementAttributeActivePowerMaxID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMaxID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000050D,
    MTRClusterElectricalMeasurementAttributeReactivePowerID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactivePowerID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000050E,
    MTRClusterElectricalMeasurementAttributeApparentPowerID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeApparentPowerID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000050F,
    MTRClusterElectricalMeasurementAttributePowerFactorID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerFactorID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000510,
    MTRClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000511,
    MTRClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000513,
    MTRClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000514,
    MTRClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000515,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSagPeriodID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000516,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000517,
    MTRClusterElectricalMeasurementAttributeAcVoltageMultiplierID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcVoltageMultiplierID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000600,
    MTRClusterElectricalMeasurementAttributeAcVoltageDivisorID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcVoltageDivisorID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000601,
    MTRClusterElectricalMeasurementAttributeAcCurrentMultiplierID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcCurrentMultiplierID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000602,
    MTRClusterElectricalMeasurementAttributeAcCurrentDivisorID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcCurrentDivisorID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000603,
    MTRClusterElectricalMeasurementAttributeAcPowerMultiplierID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcPowerMultiplierID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000604,
    MTRClusterElectricalMeasurementAttributeAcPowerDivisorID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcPowerDivisorID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000605,
    MTRClusterElectricalMeasurementAttributeOverloadAlarmsMaskID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeOverloadAlarmsMaskID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000700,
    MTRClusterElectricalMeasurementAttributeVoltageOverloadID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeVoltageOverloadID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000701,
    MTRClusterElectricalMeasurementAttributeCurrentOverloadID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeCurrentOverloadID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000702,
    MTRClusterElectricalMeasurementAttributeAcOverloadAlarmsMaskID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcOverloadAlarmsMaskID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000800,
    MTRClusterElectricalMeasurementAttributeAcVoltageOverloadID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcVoltageOverloadID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000801,
    MTRClusterElectricalMeasurementAttributeAcCurrentOverloadID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcCurrentOverloadID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000802,
    MTRClusterElectricalMeasurementAttributeAcActivePowerOverloadID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcActivePowerOverloadID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000803,
    MTRClusterElectricalMeasurementAttributeAcReactivePowerOverloadID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcReactivePowerOverloadID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000804,
    MTRClusterElectricalMeasurementAttributeAverageRmsOverVoltageID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsOverVoltageID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000805,
    MTRClusterElectricalMeasurementAttributeAverageRmsUnderVoltageID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000806,
    MTRClusterElectricalMeasurementAttributeRmsExtremeOverVoltageID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltageID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000807,
    MTRClusterElectricalMeasurementAttributeRmsExtremeUnderVoltageID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltageID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000808,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSagID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000809,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSwellID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000080A,
    MTRClusterElectricalMeasurementAttributeLineCurrentPhaseBID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeLineCurrentPhaseBID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000901,
    MTRClusterElectricalMeasurementAttributeActiveCurrentPhaseBID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActiveCurrentPhaseBID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000902,
    MTRClusterElectricalMeasurementAttributeReactiveCurrentPhaseBID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactiveCurrentPhaseBID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000903,
    MTRClusterElectricalMeasurementAttributeRmsVoltagePhaseBID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltagePhaseBID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000905,
    MTRClusterElectricalMeasurementAttributeRmsVoltageMinPhaseBID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMinPhaseBID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000906,
    MTRClusterElectricalMeasurementAttributeRmsVoltageMaxPhaseBID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMaxPhaseBID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000907,
    MTRClusterElectricalMeasurementAttributeRmsCurrentPhaseBID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentPhaseBID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000908,
    MTRClusterElectricalMeasurementAttributeRmsCurrentMinPhaseBID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMinPhaseBID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000909,
    MTRClusterElectricalMeasurementAttributeRmsCurrentMaxPhaseBID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMaxPhaseBID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000090A,
    MTRClusterElectricalMeasurementAttributeActivePowerPhaseBID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerPhaseBID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000090B,
    MTRClusterElectricalMeasurementAttributeActivePowerMinPhaseBID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMinPhaseBID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000090C,
    MTRClusterElectricalMeasurementAttributeActivePowerMaxPhaseBID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMaxPhaseBID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000090D,
    MTRClusterElectricalMeasurementAttributeReactivePowerPhaseBID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactivePowerPhaseBID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000090E,
    MTRClusterElectricalMeasurementAttributeApparentPowerPhaseBID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeApparentPowerPhaseBID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000090F,
    MTRClusterElectricalMeasurementAttributePowerFactorPhaseBID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerFactorPhaseBID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000910,
    MTRClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodPhaseBID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodPhaseBID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000911,
    MTRClusterElectricalMeasurementAttributeAverageRmsOverVoltageCounterPhaseBID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsOverVoltageCounterPhaseBID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000912,
    MTRClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterPhaseBID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterPhaseBID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000913,
    MTRClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodPhaseBID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodPhaseBID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000914,
    MTRClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodPhaseBID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodPhaseBID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000915,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSagPeriodPhaseBID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodPhaseBID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000916,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodPhaseBID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodPhaseBID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000917,
    MTRClusterElectricalMeasurementAttributeLineCurrentPhaseCID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeLineCurrentPhaseCID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A01,
    MTRClusterElectricalMeasurementAttributeActiveCurrentPhaseCID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActiveCurrentPhaseCID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A02,
    MTRClusterElectricalMeasurementAttributeReactiveCurrentPhaseCID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactiveCurrentPhaseCID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A03,
    MTRClusterElectricalMeasurementAttributeRmsVoltagePhaseCID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltagePhaseCID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A05,
    MTRClusterElectricalMeasurementAttributeRmsVoltageMinPhaseCID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMinPhaseCID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A06,
    MTRClusterElectricalMeasurementAttributeRmsVoltageMaxPhaseCID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMaxPhaseCID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A07,
    MTRClusterElectricalMeasurementAttributeRmsCurrentPhaseCID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentPhaseCID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A08,
    MTRClusterElectricalMeasurementAttributeRmsCurrentMinPhaseCID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMinPhaseCID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A09,
    MTRClusterElectricalMeasurementAttributeRmsCurrentMaxPhaseCID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMaxPhaseCID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A0A,
    MTRClusterElectricalMeasurementAttributeActivePowerPhaseCID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerPhaseCID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A0B,
    MTRClusterElectricalMeasurementAttributeActivePowerMinPhaseCID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMinPhaseCID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A0C,
    MTRClusterElectricalMeasurementAttributeActivePowerMaxPhaseCID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMaxPhaseCID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A0D,
    MTRClusterElectricalMeasurementAttributeReactivePowerPhaseCID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactivePowerPhaseCID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A0E,
    MTRClusterElectricalMeasurementAttributeApparentPowerPhaseCID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeApparentPowerPhaseCID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A0F,
    MTRClusterElectricalMeasurementAttributePowerFactorPhaseCID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerFactorPhaseCID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A10,
    MTRClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodPhaseCID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodPhaseCID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A11,
    MTRClusterElectricalMeasurementAttributeAverageRmsOverVoltageCounterPhaseCID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsOverVoltageCounterPhaseCID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A12,
    MTRClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterPhaseCID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterPhaseCID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A13,
    MTRClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodPhaseCID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodPhaseCID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A14,
    MTRClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodPhaseCID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodPhaseCID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A15,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSagPeriodPhaseCID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodPhaseCID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A16,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodPhaseCID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodPhaseCID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A17,
    MTRClusterElectricalMeasurementAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeGeneratedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterElectricalMeasurementAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcceptedCommandListID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterElectricalMeasurementAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterElectricalMeasurementAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterElectricalMeasurementAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ElectricalMeasurement attributes
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasurementTypeID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000100,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageMinID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000101,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageMaxID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000102,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000103,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentMinID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000104,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentMaxID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000105,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000106,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerMinID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000107,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerMaxID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000108,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageMultiplierID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000200,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageDivisorID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000201,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentMultiplierID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000202,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentDivisorID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000203,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerMultiplierID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000204,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerDivisorID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000205,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000300,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyMinID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000301,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyMaxID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000302,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeNeutralCurrentID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000303,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeTotalActivePowerID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000304,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeTotalReactivePowerID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000305,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeTotalApparentPowerID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000306,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured1stHarmonicCurrentID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000307,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured3rdHarmonicCurrentID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000308,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured5thHarmonicCurrentID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000309,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured7thHarmonicCurrentID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000030A,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured9thHarmonicCurrentID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000030B,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured11thHarmonicCurrentID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000030C,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase1stHarmonicCurrentID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000030D,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase3rdHarmonicCurrentID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000030E,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase5thHarmonicCurrentID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000030F,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase7thHarmonicCurrentID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000310,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase9thHarmonicCurrentID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000311,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase11thHarmonicCurrentID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000312,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyMultiplierID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000400,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyDivisorID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000401,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerMultiplierID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000402,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerDivisorID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000403,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeHarmonicCurrentMultiplierID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000404,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributePhaseHarmonicCurrentMultiplierID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000405,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousVoltageID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000500,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousLineCurrentID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000501,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousActiveCurrentID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000502,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousReactiveCurrentID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000503,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousPowerID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000504,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000505,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMinID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000506,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMaxID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000507,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000508,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMinID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000509,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMaxID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000050A,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000050B,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMinID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000050C,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMaxID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000050D,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactivePowerID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000050E,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeApparentPowerID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000050F,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerFactorID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000510,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000511,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000513,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000514,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000515,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000516,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000517,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcVoltageMultiplierID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000600,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcVoltageDivisorID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000601,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcCurrentMultiplierID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000602,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcCurrentDivisorID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000603,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcPowerMultiplierID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000604,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcPowerDivisorID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000605,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeOverloadAlarmsMaskID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000700,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeVoltageOverloadID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000701,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeCurrentOverloadID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000702,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcOverloadAlarmsMaskID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000800,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcVoltageOverloadID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000801,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcCurrentOverloadID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000802,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcActivePowerOverloadID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000803,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcReactivePowerOverloadID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000804,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsOverVoltageID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000805,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000806,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltageID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000807,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltageID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000808,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000809,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000080A,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeLineCurrentPhaseBID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000901,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActiveCurrentPhaseBID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000902,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactiveCurrentPhaseBID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000903,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltagePhaseBID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000905,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMinPhaseBID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000906,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMaxPhaseBID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000907,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentPhaseBID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000908,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMinPhaseBID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000909,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMaxPhaseBID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000090A,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerPhaseBID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000090B,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMinPhaseBID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000090C,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMaxPhaseBID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000090D,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactivePowerPhaseBID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000090E,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeApparentPowerPhaseBID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000090F,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerFactorPhaseBID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000910,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodPhaseBID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000911,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsOverVoltageCounterPhaseBID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000912,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterPhaseBID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000913,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodPhaseBID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000914,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodPhaseBID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000915,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodPhaseBID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000916,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodPhaseBID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000917,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeLineCurrentPhaseCID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000A01,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActiveCurrentPhaseCID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000A02,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactiveCurrentPhaseCID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000A03,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltagePhaseCID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000A05,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMinPhaseCID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000A06,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMaxPhaseCID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000A07,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentPhaseCID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000A08,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMinPhaseCID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000A09,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMaxPhaseCID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000A0A,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerPhaseCID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000A0B,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMinPhaseCID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000A0C,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMaxPhaseCID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000A0D,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactivePowerPhaseCID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000A0E,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeApparentPowerPhaseCID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000A0F,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerFactorPhaseCID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000A10,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodPhaseCID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000A11,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsOverVoltageCounterPhaseCID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000A12,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterPhaseCID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000A13,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodPhaseCID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000A14,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodPhaseCID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000A15,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodPhaseCID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000A16,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodPhaseCID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000A17,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAttributeListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeFeatureMapID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster TestCluster deprecated attribute names
    MTRClusterTestClusterAttributeBooleanID API_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeBooleanID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterTestClusterAttributeBitmap8ID API_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeBitmap8ID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterTestClusterAttributeBitmap16ID API_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeBitmap16ID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterTestClusterAttributeBitmap32ID API_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeBitmap32ID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterTestClusterAttributeBitmap64ID API_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeBitmap64ID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterTestClusterAttributeInt8uID API_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt8uID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterTestClusterAttributeInt16uID API_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt16uID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterTestClusterAttributeInt24uID API_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt24uID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterTestClusterAttributeInt32uID API_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt32uID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterTestClusterAttributeInt40uID API_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt40uID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterTestClusterAttributeInt48uID API_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt48uID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterTestClusterAttributeInt56uID API_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt56uID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000B,
    MTRClusterTestClusterAttributeInt64uID API_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt64uID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000C,
    MTRClusterTestClusterAttributeInt8sID API_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt8sID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000D,
    MTRClusterTestClusterAttributeInt16sID API_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt16sID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000E,
    MTRClusterTestClusterAttributeInt24sID API_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt24sID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000F,
    MTRClusterTestClusterAttributeInt32sID API_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt32sID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,
    MTRClusterTestClusterAttributeInt40sID API_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt40sID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterTestClusterAttributeInt48sID API_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt48sID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterTestClusterAttributeInt56sID API_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt56sID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000013,
    MTRClusterTestClusterAttributeInt64sID API_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt64sID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000014,
    MTRClusterTestClusterAttributeEnum8ID API_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeEnum8ID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000015,
    MTRClusterTestClusterAttributeEnum16ID API_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeEnum16ID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000016,
    MTRClusterTestClusterAttributeFloatSingleID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeFloatSingleID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000017,
    MTRClusterTestClusterAttributeFloatDoubleID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeFloatDoubleID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000018,
    MTRClusterTestClusterAttributeOctetStringID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeOctetStringID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000019,
    MTRClusterTestClusterAttributeListInt8uID API_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeListInt8uID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001A,
    MTRClusterTestClusterAttributeListOctetStringID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeListOctetStringID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001B,
    MTRClusterTestClusterAttributeListStructOctetStringID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeListStructOctetStringID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001C,
    MTRClusterTestClusterAttributeLongOctetStringID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeLongOctetStringID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001D,
    MTRClusterTestClusterAttributeCharStringID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeCharStringID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001E,
    MTRClusterTestClusterAttributeLongCharStringID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeLongCharStringID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001F,
    MTRClusterTestClusterAttributeEpochUsID API_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeEpochUsID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000020,
    MTRClusterTestClusterAttributeEpochSID API_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeEpochSID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000021,
    MTRClusterTestClusterAttributeVendorIdID API_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeVendorIdID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000022,
    MTRClusterTestClusterAttributeListNullablesAndOptionalsStructID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeListNullablesAndOptionalsStructID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000023,
    MTRClusterTestClusterAttributeEnumAttrID API_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeEnumAttrID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000024,
    MTRClusterTestClusterAttributeStructAttrID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeStructAttrID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000025,
    MTRClusterTestClusterAttributeRangeRestrictedInt8uID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt8uID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000026,
    MTRClusterTestClusterAttributeRangeRestrictedInt8sID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt8sID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000027,
    MTRClusterTestClusterAttributeRangeRestrictedInt16uID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt16uID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000028,
    MTRClusterTestClusterAttributeRangeRestrictedInt16sID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt16sID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000029,
    MTRClusterTestClusterAttributeListLongOctetStringID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeListLongOctetStringID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002A,
    MTRClusterTestClusterAttributeListFabricScopedID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeListFabricScopedID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002B,
    MTRClusterTestClusterAttributeTimedWriteBooleanID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeTimedWriteBooleanID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000030,
    MTRClusterTestClusterAttributeGeneralErrorBooleanID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeGeneralErrorBooleanID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000031,
    MTRClusterTestClusterAttributeClusterErrorBooleanID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeClusterErrorBooleanID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000032,
    MTRClusterTestClusterAttributeUnsupportedID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeUnsupportedID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x000000FF,
    MTRClusterTestClusterAttributeNullableBooleanID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableBooleanID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004000,
    MTRClusterTestClusterAttributeNullableBitmap8ID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap8ID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004001,
    MTRClusterTestClusterAttributeNullableBitmap16ID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap16ID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004002,
    MTRClusterTestClusterAttributeNullableBitmap32ID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap32ID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004003,
    MTRClusterTestClusterAttributeNullableBitmap64ID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap64ID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004004,
    MTRClusterTestClusterAttributeNullableInt8uID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt8uID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004005,
    MTRClusterTestClusterAttributeNullableInt16uID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt16uID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004006,
    MTRClusterTestClusterAttributeNullableInt24uID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt24uID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004007,
    MTRClusterTestClusterAttributeNullableInt32uID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt32uID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004008,
    MTRClusterTestClusterAttributeNullableInt40uID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt40uID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004009,
    MTRClusterTestClusterAttributeNullableInt48uID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt48uID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000400A,
    MTRClusterTestClusterAttributeNullableInt56uID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt56uID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000400B,
    MTRClusterTestClusterAttributeNullableInt64uID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt64uID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000400C,
    MTRClusterTestClusterAttributeNullableInt8sID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt8sID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000400D,
    MTRClusterTestClusterAttributeNullableInt16sID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt16sID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000400E,
    MTRClusterTestClusterAttributeNullableInt24sID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt24sID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000400F,
    MTRClusterTestClusterAttributeNullableInt32sID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt32sID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004010,
    MTRClusterTestClusterAttributeNullableInt40sID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt40sID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004011,
    MTRClusterTestClusterAttributeNullableInt48sID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt48sID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004012,
    MTRClusterTestClusterAttributeNullableInt56sID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt56sID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004013,
    MTRClusterTestClusterAttributeNullableInt64sID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt64sID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004014,
    MTRClusterTestClusterAttributeNullableEnum8ID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableEnum8ID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004015,
    MTRClusterTestClusterAttributeNullableEnum16ID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableEnum16ID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004016,
    MTRClusterTestClusterAttributeNullableFloatSingleID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableFloatSingleID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004017,
    MTRClusterTestClusterAttributeNullableFloatDoubleID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableFloatDoubleID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004018,
    MTRClusterTestClusterAttributeNullableOctetStringID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableOctetStringID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004019,
    MTRClusterTestClusterAttributeNullableCharStringID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableCharStringID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000401E,
    MTRClusterTestClusterAttributeNullableEnumAttrID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableEnumAttrID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004024,
    MTRClusterTestClusterAttributeNullableStructID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableStructID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004025,
    MTRClusterTestClusterAttributeNullableRangeRestrictedInt8uID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt8uID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004026,
    MTRClusterTestClusterAttributeNullableRangeRestrictedInt8sID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt8sID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004027,
    MTRClusterTestClusterAttributeNullableRangeRestrictedInt16uID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt16uID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004028,
    MTRClusterTestClusterAttributeNullableRangeRestrictedInt16sID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt16sID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004029,
    MTRClusterTestClusterAttributeWriteOnlyInt8uID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeWriteOnlyInt8uID", ios(16.2, 16.4), macos(13.1, 13.3),
        watchos(9.2, 9.4), tvos(16.2, 16.4))
    = 0x0000402A,
    MTRClusterTestClusterAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterTestClusterAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterTestClusterAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterTestClusterAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterTestClusterAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster UnitTesting attributes
    MTRAttributeIDTypeClusterUnitTestingAttributeBooleanID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRAttributeIDTypeClusterUnitTestingAttributeBitmap8ID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRAttributeIDTypeClusterUnitTestingAttributeBitmap16ID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRAttributeIDTypeClusterUnitTestingAttributeBitmap32ID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRAttributeIDTypeClusterUnitTestingAttributeBitmap64ID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt8uID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt16uID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt24uID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000007,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt32uID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000008,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt40uID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000009,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt48uID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000A,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt56uID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000B,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt64uID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000C,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt8sID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000D,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt16sID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000E,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt24sID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000F,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt32sID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000010,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt40sID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000011,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt48sID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000012,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt56sID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000013,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt64sID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000014,
    MTRAttributeIDTypeClusterUnitTestingAttributeEnum8ID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000015,
    MTRAttributeIDTypeClusterUnitTestingAttributeEnum16ID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000016,
    MTRAttributeIDTypeClusterUnitTestingAttributeFloatSingleID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000017,
    MTRAttributeIDTypeClusterUnitTestingAttributeFloatDoubleID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000018,
    MTRAttributeIDTypeClusterUnitTestingAttributeOctetStringID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000019,
    MTRAttributeIDTypeClusterUnitTestingAttributeListInt8uID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000001A,
    MTRAttributeIDTypeClusterUnitTestingAttributeListOctetStringID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000001B,
    MTRAttributeIDTypeClusterUnitTestingAttributeListStructOctetStringID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000001C,
    MTRAttributeIDTypeClusterUnitTestingAttributeLongOctetStringID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000001D,
    MTRAttributeIDTypeClusterUnitTestingAttributeCharStringID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000001E,
    MTRAttributeIDTypeClusterUnitTestingAttributeLongCharStringID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000001F,
    MTRAttributeIDTypeClusterUnitTestingAttributeEpochUsID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000020,
    MTRAttributeIDTypeClusterUnitTestingAttributeEpochSID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000021,
    MTRAttributeIDTypeClusterUnitTestingAttributeVendorIdID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000022,
    MTRAttributeIDTypeClusterUnitTestingAttributeListNullablesAndOptionalsStructID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000023,
    MTRAttributeIDTypeClusterUnitTestingAttributeEnumAttrID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000024,
    MTRAttributeIDTypeClusterUnitTestingAttributeStructAttrID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000025,
    MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt8uID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000026,
    MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt8sID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000027,
    MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt16uID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000028,
    MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt16sID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000029,
    MTRAttributeIDTypeClusterUnitTestingAttributeListLongOctetStringID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000002A,
    MTRAttributeIDTypeClusterUnitTestingAttributeListFabricScopedID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000002B,
    MTRAttributeIDTypeClusterUnitTestingAttributeTimedWriteBooleanID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000030,
    MTRAttributeIDTypeClusterUnitTestingAttributeGeneralErrorBooleanID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000031,
    MTRAttributeIDTypeClusterUnitTestingAttributeClusterErrorBooleanID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000032,
    MTRAttributeIDTypeClusterUnitTestingAttributeUnsupportedID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x000000FF,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableBooleanID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004000,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap8ID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004001,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap16ID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004002,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap32ID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004003,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap64ID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004004,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt8uID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004005,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt16uID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004006,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt24uID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004007,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt32uID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004008,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt40uID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004009,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt48uID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000400A,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt56uID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000400B,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt64uID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000400C,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt8sID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000400D,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt16sID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000400E,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt24sID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000400F,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt32sID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004010,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt40sID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004011,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt48sID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004012,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt56sID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004013,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt64sID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004014,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableEnum8ID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004015,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableEnum16ID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004016,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableFloatSingleID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004017,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableFloatDoubleID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004018,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableOctetStringID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004019,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableCharStringID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000401E,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableEnumAttrID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004024,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableStructID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004025,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt8uID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004026,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt8sID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004027,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt16uID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004028,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt16sID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00004029,
    MTRAttributeIDTypeClusterUnitTestingAttributeWriteOnlyInt8uID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000402A,
    MTRAttributeIDTypeClusterUnitTestingAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterUnitTestingAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterUnitTestingAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterUnitTestingAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterUnitTestingAttributeClusterRevisionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster FaultInjection deprecated attribute names
    MTRClusterFaultInjectionAttributeGeneratedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFaultInjectionAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterFaultInjectionAttributeAcceptedCommandListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFaultInjectionAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterFaultInjectionAttributeAttributeListID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFaultInjectionAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterFaultInjectionAttributeFeatureMapID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFaultInjectionAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterFaultInjectionAttributeClusterRevisionID API_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFaultInjectionAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster FaultInjection attributes
    MTRAttributeIDTypeClusterFaultInjectionAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterFaultInjectionAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterFaultInjectionAttributeAttributeListID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterFaultInjectionAttributeFeatureMapID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterFaultInjectionAttributeClusterRevisionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

};

#pragma mark - Commands IDs

typedef NS_ENUM(uint32_t, MTRCommandIDType) {

    // Cluster Identify deprecated command id names
    MTRClusterIdentifyCommandIdentifyID API_DEPRECATED("Please use MTRCommandIDTypeClusterIdentifyCommandIdentifyID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterIdentifyCommandTriggerEffectID API_DEPRECATED("Please use MTRCommandIDTypeClusterIdentifyCommandTriggerEffectID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000040,

    // Cluster Identify commands
    MTRCommandIDTypeClusterIdentifyCommandIdentifyID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterIdentifyCommandTriggerEffectID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000040,

    // Cluster Groups deprecated command id names
    MTRClusterGroupsCommandAddGroupID API_DEPRECATED("Please use MTRCommandIDTypeClusterGroupsCommandAddGroupID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterGroupsCommandAddGroupResponseID API_DEPRECATED("Please use MTRCommandIDTypeClusterGroupsCommandAddGroupResponseID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterGroupsCommandViewGroupID API_DEPRECATED("Please use MTRCommandIDTypeClusterGroupsCommandViewGroupID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterGroupsCommandViewGroupResponseID API_DEPRECATED("Please use MTRCommandIDTypeClusterGroupsCommandViewGroupResponseID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterGroupsCommandGetGroupMembershipID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGroupsCommandGetGroupMembershipID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterGroupsCommandGetGroupMembershipResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGroupsCommandGetGroupMembershipResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterGroupsCommandRemoveGroupID API_DEPRECATED("Please use MTRCommandIDTypeClusterGroupsCommandRemoveGroupID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterGroupsCommandRemoveGroupResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGroupsCommandRemoveGroupResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterGroupsCommandRemoveAllGroupsID API_DEPRECATED("Please use MTRCommandIDTypeClusterGroupsCommandRemoveAllGroupsID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterGroupsCommandAddGroupIfIdentifyingID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGroupsCommandAddGroupIfIdentifyingID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,

    // Cluster Groups commands
    MTRCommandIDTypeClusterGroupsCommandAddGroupID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterGroupsCommandAddGroupResponseID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRCommandIDTypeClusterGroupsCommandViewGroupID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterGroupsCommandViewGroupResponseID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRCommandIDTypeClusterGroupsCommandGetGroupMembershipID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRCommandIDTypeClusterGroupsCommandGetGroupMembershipResponseID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRCommandIDTypeClusterGroupsCommandRemoveGroupID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRCommandIDTypeClusterGroupsCommandRemoveGroupResponseID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRCommandIDTypeClusterGroupsCommandRemoveAllGroupsID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRCommandIDTypeClusterGroupsCommandAddGroupIfIdentifyingID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,

    // Cluster Scenes deprecated command id names
    MTRClusterScenesCommandAddSceneID API_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandAddSceneID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterScenesCommandAddSceneResponseID API_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandAddSceneResponseID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterScenesCommandViewSceneID API_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandViewSceneID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterScenesCommandViewSceneResponseID API_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandViewSceneResponseID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterScenesCommandRemoveSceneID API_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandRemoveSceneID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterScenesCommandRemoveSceneResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterScenesCommandRemoveSceneResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterScenesCommandRemoveAllScenesID API_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandRemoveAllScenesID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterScenesCommandRemoveAllScenesResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterScenesCommandRemoveAllScenesResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterScenesCommandStoreSceneID API_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandStoreSceneID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterScenesCommandStoreSceneResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterScenesCommandStoreSceneResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterScenesCommandRecallSceneID API_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandRecallSceneID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterScenesCommandGetSceneMembershipID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterScenesCommandGetSceneMembershipID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterScenesCommandGetSceneMembershipResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterScenesCommandGetSceneMembershipResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterScenesCommandEnhancedAddSceneID API_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandEnhancedAddSceneID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000040,
    MTRClusterScenesCommandEnhancedAddSceneResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterScenesCommandEnhancedAddSceneResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000040,
    MTRClusterScenesCommandEnhancedViewSceneID API_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandEnhancedViewSceneID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000041,
    MTRClusterScenesCommandEnhancedViewSceneResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterScenesCommandEnhancedViewSceneResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000041,
    MTRClusterScenesCommandCopySceneID API_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandCopySceneID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000042,
    MTRClusterScenesCommandCopySceneResponseID API_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandCopySceneResponseID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000042,

    // Cluster Scenes commands
    MTRCommandIDTypeClusterScenesCommandAddSceneID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterScenesCommandAddSceneResponseID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRCommandIDTypeClusterScenesCommandViewSceneID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterScenesCommandViewSceneResponseID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRCommandIDTypeClusterScenesCommandRemoveSceneID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRCommandIDTypeClusterScenesCommandRemoveSceneResponseID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRCommandIDTypeClusterScenesCommandRemoveAllScenesID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRCommandIDTypeClusterScenesCommandRemoveAllScenesResponseID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRCommandIDTypeClusterScenesCommandStoreSceneID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRCommandIDTypeClusterScenesCommandStoreSceneResponseID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRCommandIDTypeClusterScenesCommandRecallSceneID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRCommandIDTypeClusterScenesCommandGetSceneMembershipID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,
    MTRCommandIDTypeClusterScenesCommandGetSceneMembershipResponseID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,
    MTRCommandIDTypeClusterScenesCommandEnhancedAddSceneID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000040,
    MTRCommandIDTypeClusterScenesCommandEnhancedAddSceneResponseID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000040,
    MTRCommandIDTypeClusterScenesCommandEnhancedViewSceneID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000041,
    MTRCommandIDTypeClusterScenesCommandEnhancedViewSceneResponseID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000041,
    MTRCommandIDTypeClusterScenesCommandCopySceneID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000042,
    MTRCommandIDTypeClusterScenesCommandCopySceneResponseID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000042,

    // Cluster OnOff deprecated command id names
    MTRClusterOnOffCommandOffID API_DEPRECATED("Please use MTRCommandIDTypeClusterOnOffCommandOffID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterOnOffCommandOnID API_DEPRECATED("Please use MTRCommandIDTypeClusterOnOffCommandOnID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterOnOffCommandToggleID API_DEPRECATED("Please use MTRCommandIDTypeClusterOnOffCommandToggleID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterOnOffCommandOffWithEffectID API_DEPRECATED("Please use MTRCommandIDTypeClusterOnOffCommandOffWithEffectID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000040,
    MTRClusterOnOffCommandOnWithRecallGlobalSceneID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOnOffCommandOnWithRecallGlobalSceneID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000041,
    MTRClusterOnOffCommandOnWithTimedOffID API_DEPRECATED("Please use MTRCommandIDTypeClusterOnOffCommandOnWithTimedOffID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000042,

    // Cluster OnOff commands
    MTRCommandIDTypeClusterOnOffCommandOffID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterOnOffCommandOnID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterOnOffCommandToggleID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRCommandIDTypeClusterOnOffCommandOffWithEffectID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000040,
    MTRCommandIDTypeClusterOnOffCommandOnWithRecallGlobalSceneID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000041,
    MTRCommandIDTypeClusterOnOffCommandOnWithTimedOffID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000042,

    // Cluster LevelControl deprecated command id names
    MTRClusterLevelControlCommandMoveToLevelID API_DEPRECATED("Please use MTRCommandIDTypeClusterLevelControlCommandMoveToLevelID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterLevelControlCommandMoveID API_DEPRECATED("Please use MTRCommandIDTypeClusterLevelControlCommandMoveID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterLevelControlCommandStepID API_DEPRECATED("Please use MTRCommandIDTypeClusterLevelControlCommandStepID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterLevelControlCommandStopID API_DEPRECATED("Please use MTRCommandIDTypeClusterLevelControlCommandStopID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterLevelControlCommandMoveToLevelWithOnOffID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterLevelControlCommandMoveToLevelWithOnOffID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterLevelControlCommandMoveWithOnOffID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterLevelControlCommandMoveWithOnOffID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterLevelControlCommandStepWithOnOffID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterLevelControlCommandStepWithOnOffID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterLevelControlCommandStopWithOnOffID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterLevelControlCommandStopWithOnOffID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterLevelControlCommandMoveToClosestFrequencyID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterLevelControlCommandMoveToClosestFrequencyID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,

    // Cluster LevelControl commands
    MTRCommandIDTypeClusterLevelControlCommandMoveToLevelID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRCommandIDTypeClusterLevelControlCommandMoveID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterLevelControlCommandStepID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRCommandIDTypeClusterLevelControlCommandStopID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRCommandIDTypeClusterLevelControlCommandMoveToLevelWithOnOffID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRCommandIDTypeClusterLevelControlCommandMoveWithOnOffID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTRCommandIDTypeClusterLevelControlCommandStepWithOnOffID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,
    MTRCommandIDTypeClusterLevelControlCommandStopWithOnOffID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000007,
    MTRCommandIDTypeClusterLevelControlCommandMoveToClosestFrequencyID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000008,

    // Cluster Actions deprecated command id names
    MTRClusterActionsCommandInstantActionID API_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandInstantActionID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterActionsCommandInstantActionWithTransitionID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterActionsCommandInstantActionWithTransitionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterActionsCommandStartActionID API_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandStartActionID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterActionsCommandStartActionWithDurationID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterActionsCommandStartActionWithDurationID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterActionsCommandStopActionID API_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandStopActionID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterActionsCommandPauseActionID API_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandPauseActionID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterActionsCommandPauseActionWithDurationID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterActionsCommandPauseActionWithDurationID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterActionsCommandResumeActionID API_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandResumeActionID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterActionsCommandEnableActionID API_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandEnableActionID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterActionsCommandEnableActionWithDurationID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterActionsCommandEnableActionWithDurationID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterActionsCommandDisableActionID API_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandDisableActionID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterActionsCommandDisableActionWithDurationID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterActionsCommandDisableActionWithDurationID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000B,

    // Cluster Actions commands
    MTRCommandIDTypeClusterActionsCommandInstantActionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRCommandIDTypeClusterActionsCommandInstantActionWithTransitionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRCommandIDTypeClusterActionsCommandStartActionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRCommandIDTypeClusterActionsCommandStartActionWithDurationID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRCommandIDTypeClusterActionsCommandStopActionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRCommandIDTypeClusterActionsCommandPauseActionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRCommandIDTypeClusterActionsCommandPauseActionWithDurationID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,
    MTRCommandIDTypeClusterActionsCommandResumeActionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000007,
    MTRCommandIDTypeClusterActionsCommandEnableActionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000008,
    MTRCommandIDTypeClusterActionsCommandEnableActionWithDurationID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000009,
    MTRCommandIDTypeClusterActionsCommandDisableActionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000A,
    MTRCommandIDTypeClusterActionsCommandDisableActionWithDurationID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000B,

    // Cluster Basic deprecated command id names
    MTRClusterBasicCommandMfgSpecificPingID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterBasicInformationCommandMfgSpecificPingID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x10020000,

    // Cluster BasicInformation commands

    // Cluster OtaSoftwareUpdateProvider deprecated command id names
    MTRClusterOtaSoftwareUpdateProviderCommandQueryImageID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandQueryImageID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterOtaSoftwareUpdateProviderCommandQueryImageResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandQueryImageResponseID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterOtaSoftwareUpdateProviderCommandApplyUpdateRequestID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandApplyUpdateRequestID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterOtaSoftwareUpdateProviderCommandApplyUpdateResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandApplyUpdateResponseID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterOtaSoftwareUpdateProviderCommandNotifyUpdateAppliedID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandNotifyUpdateAppliedID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,

    // Cluster OTASoftwareUpdateProvider commands
    MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandQueryImageID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandQueryImageResponseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandApplyUpdateRequestID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandApplyUpdateResponseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandNotifyUpdateAppliedID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,

    // Cluster OtaSoftwareUpdateRequestor deprecated command id names
    MTRClusterOtaSoftwareUpdateRequestorCommandAnnounceOtaProviderID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOTASoftwareUpdateRequestorCommandAnnounceOTAProviderID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,

    // Cluster OTASoftwareUpdateRequestor commands
    MTRCommandIDTypeClusterOTASoftwareUpdateRequestorCommandAnnounceOTAProviderID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,

    // Cluster GeneralCommissioning deprecated command id names
    MTRClusterGeneralCommissioningCommandArmFailSafeID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGeneralCommissioningCommandArmFailSafeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterGeneralCommissioningCommandArmFailSafeResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGeneralCommissioningCommandArmFailSafeResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterGeneralCommissioningCommandSetRegulatoryConfigID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGeneralCommissioningCommandSetRegulatoryConfigID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterGeneralCommissioningCommandSetRegulatoryConfigResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGeneralCommissioningCommandSetRegulatoryConfigResponseID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterGeneralCommissioningCommandCommissioningCompleteID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGeneralCommissioningCommandCommissioningCompleteID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterGeneralCommissioningCommandCommissioningCompleteResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGeneralCommissioningCommandCommissioningCompleteResponseID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,

    // Cluster GeneralCommissioning commands
    MTRCommandIDTypeClusterGeneralCommissioningCommandArmFailSafeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRCommandIDTypeClusterGeneralCommissioningCommandArmFailSafeResponseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRCommandIDTypeClusterGeneralCommissioningCommandSetRegulatoryConfigID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRCommandIDTypeClusterGeneralCommissioningCommandSetRegulatoryConfigResponseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRCommandIDTypeClusterGeneralCommissioningCommandCommissioningCompleteID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRCommandIDTypeClusterGeneralCommissioningCommandCommissioningCompleteResponseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,

    // Cluster NetworkCommissioning deprecated command id names
    MTRClusterNetworkCommissioningCommandScanNetworksID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterNetworkCommissioningCommandScanNetworksID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterNetworkCommissioningCommandScanNetworksResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterNetworkCommissioningCommandScanNetworksResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterNetworkCommissioningCommandAddOrUpdateWiFiNetworkID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterNetworkCommissioningCommandAddOrUpdateWiFiNetworkID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterNetworkCommissioningCommandAddOrUpdateThreadNetworkID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterNetworkCommissioningCommandAddOrUpdateThreadNetworkID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterNetworkCommissioningCommandRemoveNetworkID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterNetworkCommissioningCommandRemoveNetworkID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterNetworkCommissioningCommandNetworkConfigResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterNetworkCommissioningCommandNetworkConfigResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterNetworkCommissioningCommandConnectNetworkID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterNetworkCommissioningCommandConnectNetworkID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterNetworkCommissioningCommandConnectNetworkResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterNetworkCommissioningCommandConnectNetworkResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterNetworkCommissioningCommandReorderNetworkID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterNetworkCommissioningCommandReorderNetworkID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,

    // Cluster NetworkCommissioning commands
    MTRCommandIDTypeClusterNetworkCommissioningCommandScanNetworksID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRCommandIDTypeClusterNetworkCommissioningCommandScanNetworksResponseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRCommandIDTypeClusterNetworkCommissioningCommandAddOrUpdateWiFiNetworkID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRCommandIDTypeClusterNetworkCommissioningCommandAddOrUpdateThreadNetworkID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRCommandIDTypeClusterNetworkCommissioningCommandRemoveNetworkID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRCommandIDTypeClusterNetworkCommissioningCommandNetworkConfigResponseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTRCommandIDTypeClusterNetworkCommissioningCommandConnectNetworkID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,
    MTRCommandIDTypeClusterNetworkCommissioningCommandConnectNetworkResponseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000007,
    MTRCommandIDTypeClusterNetworkCommissioningCommandReorderNetworkID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000008,

    // Cluster DiagnosticLogs deprecated command id names
    MTRClusterDiagnosticLogsCommandRetrieveLogsRequestID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDiagnosticLogsCommandRetrieveLogsRequestID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterDiagnosticLogsCommandRetrieveLogsResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDiagnosticLogsCommandRetrieveLogsResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,

    // Cluster DiagnosticLogs commands
    MTRCommandIDTypeClusterDiagnosticLogsCommandRetrieveLogsRequestID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRCommandIDTypeClusterDiagnosticLogsCommandRetrieveLogsResponseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,

    // Cluster GeneralDiagnostics deprecated command id names
    MTRClusterGeneralDiagnosticsCommandTestEventTriggerID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGeneralDiagnosticsCommandTestEventTriggerID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,

    // Cluster GeneralDiagnostics commands
    MTRCommandIDTypeClusterGeneralDiagnosticsCommandTestEventTriggerID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,

    // Cluster SoftwareDiagnostics deprecated command id names
    MTRClusterSoftwareDiagnosticsCommandResetWatermarksID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterSoftwareDiagnosticsCommandResetWatermarksID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,

    // Cluster SoftwareDiagnostics commands
    MTRCommandIDTypeClusterSoftwareDiagnosticsCommandResetWatermarksID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,

    // Cluster ThreadNetworkDiagnostics deprecated command id names
    MTRClusterThreadNetworkDiagnosticsCommandResetCountsID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterThreadNetworkDiagnosticsCommandResetCountsID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,

    // Cluster ThreadNetworkDiagnostics commands
    MTRCommandIDTypeClusterThreadNetworkDiagnosticsCommandResetCountsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,

    // Cluster WiFiNetworkDiagnostics deprecated command id names
    MTRClusterWiFiNetworkDiagnosticsCommandResetCountsID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterWiFiNetworkDiagnosticsCommandResetCountsID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,

    // Cluster WiFiNetworkDiagnostics commands
    MTRCommandIDTypeClusterWiFiNetworkDiagnosticsCommandResetCountsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,

    // Cluster EthernetNetworkDiagnostics deprecated command id names
    MTRClusterEthernetNetworkDiagnosticsCommandResetCountsID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterEthernetNetworkDiagnosticsCommandResetCountsID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,

    // Cluster EthernetNetworkDiagnostics commands
    MTRCommandIDTypeClusterEthernetNetworkDiagnosticsCommandResetCountsID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,

    // Cluster TimeSynchronization deprecated command id names
    MTRClusterTimeSynchronizationCommandSetUtcTimeID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterTimeSynchronizationCommandSetUtcTimeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,

    // Cluster TimeSynchronization commands
    MTRCommandIDTypeClusterTimeSynchronizationCommandSetUtcTimeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,

    // Cluster AdministratorCommissioning deprecated command id names
    MTRClusterAdministratorCommissioningCommandOpenCommissioningWindowID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterAdministratorCommissioningCommandOpenCommissioningWindowID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterAdministratorCommissioningCommandOpenBasicCommissioningWindowID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterAdministratorCommissioningCommandOpenBasicCommissioningWindowID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterAdministratorCommissioningCommandRevokeCommissioningID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterAdministratorCommissioningCommandRevokeCommissioningID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,

    // Cluster AdministratorCommissioning commands
    MTRCommandIDTypeClusterAdministratorCommissioningCommandOpenCommissioningWindowID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRCommandIDTypeClusterAdministratorCommissioningCommandOpenBasicCommissioningWindowID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRCommandIDTypeClusterAdministratorCommissioningCommandRevokeCommissioningID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,

    // Cluster OperationalCredentials deprecated command id names
    MTRClusterOperationalCredentialsCommandAttestationRequestID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOperationalCredentialsCommandAttestationRequestID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterOperationalCredentialsCommandAttestationResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOperationalCredentialsCommandAttestationResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterOperationalCredentialsCommandCertificateChainRequestID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOperationalCredentialsCommandCertificateChainRequestID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterOperationalCredentialsCommandCertificateChainResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOperationalCredentialsCommandCertificateChainResponseID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterOperationalCredentialsCommandCSRRequestID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOperationalCredentialsCommandCSRRequestID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterOperationalCredentialsCommandCSRResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOperationalCredentialsCommandCSRResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterOperationalCredentialsCommandAddNOCID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOperationalCredentialsCommandAddNOCID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterOperationalCredentialsCommandUpdateNOCID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOperationalCredentialsCommandUpdateNOCID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterOperationalCredentialsCommandNOCResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOperationalCredentialsCommandNOCResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterOperationalCredentialsCommandUpdateFabricLabelID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOperationalCredentialsCommandUpdateFabricLabelID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterOperationalCredentialsCommandRemoveFabricID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOperationalCredentialsCommandRemoveFabricID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterOperationalCredentialsCommandAddTrustedRootCertificateID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOperationalCredentialsCommandAddTrustedRootCertificateID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000B,

    // Cluster OperationalCredentials commands
    MTRCommandIDTypeClusterOperationalCredentialsCommandAttestationRequestID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRCommandIDTypeClusterOperationalCredentialsCommandAttestationResponseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRCommandIDTypeClusterOperationalCredentialsCommandCertificateChainRequestID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRCommandIDTypeClusterOperationalCredentialsCommandCertificateChainResponseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRCommandIDTypeClusterOperationalCredentialsCommandCSRRequestID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRCommandIDTypeClusterOperationalCredentialsCommandCSRResponseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTRCommandIDTypeClusterOperationalCredentialsCommandAddNOCID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,
    MTRCommandIDTypeClusterOperationalCredentialsCommandUpdateNOCID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000007,
    MTRCommandIDTypeClusterOperationalCredentialsCommandNOCResponseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000008,
    MTRCommandIDTypeClusterOperationalCredentialsCommandUpdateFabricLabelID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000009,
    MTRCommandIDTypeClusterOperationalCredentialsCommandRemoveFabricID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000A,
    MTRCommandIDTypeClusterOperationalCredentialsCommandAddTrustedRootCertificateID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000B,

    // Cluster GroupKeyManagement deprecated command id names
    MTRClusterGroupKeyManagementCommandKeySetWriteID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetWriteID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterGroupKeyManagementCommandKeySetReadID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterGroupKeyManagementCommandKeySetReadResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterGroupKeyManagementCommandKeySetRemoveID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetRemoveID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterGroupKeyManagementCommandKeySetReadAllIndicesID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadAllIndicesID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterGroupKeyManagementCommandKeySetReadAllIndicesResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadAllIndicesResponseID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,

    // Cluster GroupKeyManagement commands
    MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetWriteID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadResponseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetRemoveID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadAllIndicesID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadAllIndicesResponseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,

    // Cluster ModeSelect deprecated command id names
    MTRClusterModeSelectCommandChangeToModeID API_DEPRECATED("Please use MTRCommandIDTypeClusterModeSelectCommandChangeToModeID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,

    // Cluster ModeSelect commands
    MTRCommandIDTypeClusterModeSelectCommandChangeToModeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,

    // Cluster DoorLock deprecated command id names
    MTRClusterDoorLockCommandLockDoorID API_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandLockDoorID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterDoorLockCommandUnlockDoorID API_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandUnlockDoorID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterDoorLockCommandUnlockWithTimeoutID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandUnlockWithTimeoutID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterDoorLockCommandSetWeekDayScheduleID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandSetWeekDayScheduleID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000B,
    MTRClusterDoorLockCommandGetWeekDayScheduleID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandGetWeekDayScheduleID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000C,
    MTRClusterDoorLockCommandGetWeekDayScheduleResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandGetWeekDayScheduleResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000C,
    MTRClusterDoorLockCommandClearWeekDayScheduleID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandClearWeekDayScheduleID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000D,
    MTRClusterDoorLockCommandSetYearDayScheduleID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandSetYearDayScheduleID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000E,
    MTRClusterDoorLockCommandGetYearDayScheduleID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandGetYearDayScheduleID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000F,
    MTRClusterDoorLockCommandGetYearDayScheduleResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandGetYearDayScheduleResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000F,
    MTRClusterDoorLockCommandClearYearDayScheduleID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandClearYearDayScheduleID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,
    MTRClusterDoorLockCommandSetHolidayScheduleID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandSetHolidayScheduleID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterDoorLockCommandGetHolidayScheduleID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandGetHolidayScheduleID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterDoorLockCommandGetHolidayScheduleResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandGetHolidayScheduleResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterDoorLockCommandClearHolidayScheduleID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandClearHolidayScheduleID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000013,
    MTRClusterDoorLockCommandSetUserID API_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandSetUserID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001A,
    MTRClusterDoorLockCommandGetUserID API_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandGetUserID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001B,
    MTRClusterDoorLockCommandGetUserResponseID API_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandGetUserResponseID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001C,
    MTRClusterDoorLockCommandClearUserID API_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandClearUserID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001D,
    MTRClusterDoorLockCommandSetCredentialID API_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandSetCredentialID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000022,
    MTRClusterDoorLockCommandSetCredentialResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandSetCredentialResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000023,
    MTRClusterDoorLockCommandGetCredentialStatusID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandGetCredentialStatusID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000024,
    MTRClusterDoorLockCommandGetCredentialStatusResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandGetCredentialStatusResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000025,
    MTRClusterDoorLockCommandClearCredentialID API_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandClearCredentialID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000026,

    // Cluster DoorLock commands
    MTRCommandIDTypeClusterDoorLockCommandLockDoorID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterDoorLockCommandUnlockDoorID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterDoorLockCommandUnlockWithTimeoutID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRCommandIDTypeClusterDoorLockCommandSetWeekDayScheduleID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000B,
    MTRCommandIDTypeClusterDoorLockCommandGetWeekDayScheduleID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000C,
    MTRCommandIDTypeClusterDoorLockCommandGetWeekDayScheduleResponseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000C,
    MTRCommandIDTypeClusterDoorLockCommandClearWeekDayScheduleID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000D,
    MTRCommandIDTypeClusterDoorLockCommandSetYearDayScheduleID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000E,
    MTRCommandIDTypeClusterDoorLockCommandGetYearDayScheduleID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000F,
    MTRCommandIDTypeClusterDoorLockCommandGetYearDayScheduleResponseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000F,
    MTRCommandIDTypeClusterDoorLockCommandClearYearDayScheduleID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000010,
    MTRCommandIDTypeClusterDoorLockCommandSetHolidayScheduleID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000011,
    MTRCommandIDTypeClusterDoorLockCommandGetHolidayScheduleID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000012,
    MTRCommandIDTypeClusterDoorLockCommandGetHolidayScheduleResponseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000012,
    MTRCommandIDTypeClusterDoorLockCommandClearHolidayScheduleID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000013,
    MTRCommandIDTypeClusterDoorLockCommandSetUserID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001A,
    MTRCommandIDTypeClusterDoorLockCommandGetUserID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001B,
    MTRCommandIDTypeClusterDoorLockCommandGetUserResponseID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000001C,
    MTRCommandIDTypeClusterDoorLockCommandClearUserID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001D,
    MTRCommandIDTypeClusterDoorLockCommandSetCredentialID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000022,
    MTRCommandIDTypeClusterDoorLockCommandSetCredentialResponseID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000023,
    MTRCommandIDTypeClusterDoorLockCommandGetCredentialStatusID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000024,
    MTRCommandIDTypeClusterDoorLockCommandGetCredentialStatusResponseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000025,
    MTRCommandIDTypeClusterDoorLockCommandClearCredentialID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000026,

    // Cluster WindowCovering deprecated command id names
    MTRClusterWindowCoveringCommandUpOrOpenID API_DEPRECATED("Please use MTRCommandIDTypeClusterWindowCoveringCommandUpOrOpenID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterWindowCoveringCommandDownOrCloseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterWindowCoveringCommandDownOrCloseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterWindowCoveringCommandStopMotionID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterWindowCoveringCommandStopMotionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterWindowCoveringCommandGoToLiftValueID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterWindowCoveringCommandGoToLiftValueID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterWindowCoveringCommandGoToLiftPercentageID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterWindowCoveringCommandGoToLiftPercentageID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterWindowCoveringCommandGoToTiltValueID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterWindowCoveringCommandGoToTiltValueID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterWindowCoveringCommandGoToTiltPercentageID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterWindowCoveringCommandGoToTiltPercentageID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,

    // Cluster WindowCovering commands
    MTRCommandIDTypeClusterWindowCoveringCommandUpOrOpenID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRCommandIDTypeClusterWindowCoveringCommandDownOrCloseID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRCommandIDTypeClusterWindowCoveringCommandStopMotionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRCommandIDTypeClusterWindowCoveringCommandGoToLiftValueID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRCommandIDTypeClusterWindowCoveringCommandGoToLiftPercentageID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTRCommandIDTypeClusterWindowCoveringCommandGoToTiltValueID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000007,
    MTRCommandIDTypeClusterWindowCoveringCommandGoToTiltPercentageID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000008,

    // Cluster BarrierControl deprecated command id names
    MTRClusterBarrierControlCommandBarrierControlGoToPercentID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterBarrierControlCommandBarrierControlGoToPercentID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterBarrierControlCommandBarrierControlStopID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterBarrierControlCommandBarrierControlStopID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,

    // Cluster BarrierControl commands
    MTRCommandIDTypeClusterBarrierControlCommandBarrierControlGoToPercentID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRCommandIDTypeClusterBarrierControlCommandBarrierControlStopID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,

    // Cluster Thermostat deprecated command id names
    MTRClusterThermostatCommandSetpointRaiseLowerID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterThermostatCommandSetpointRaiseLowerID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterThermostatCommandGetWeeklyScheduleResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterThermostatCommandGetWeeklyScheduleResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterThermostatCommandSetWeeklyScheduleID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterThermostatCommandSetWeeklyScheduleID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterThermostatCommandGetWeeklyScheduleID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterThermostatCommandGetWeeklyScheduleID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterThermostatCommandClearWeeklyScheduleID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterThermostatCommandClearWeeklyScheduleID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,

    // Cluster Thermostat commands
    MTRCommandIDTypeClusterThermostatCommandSetpointRaiseLowerID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRCommandIDTypeClusterThermostatCommandGetWeeklyScheduleResponseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRCommandIDTypeClusterThermostatCommandSetWeeklyScheduleID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRCommandIDTypeClusterThermostatCommandGetWeeklyScheduleID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRCommandIDTypeClusterThermostatCommandClearWeeklyScheduleID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,

    // Cluster ColorControl deprecated command id names
    MTRClusterColorControlCommandMoveToHueID API_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandMoveToHueID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterColorControlCommandMoveHueID API_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandMoveHueID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterColorControlCommandStepHueID API_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandStepHueID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterColorControlCommandMoveToSaturationID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterColorControlCommandMoveToSaturationID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterColorControlCommandMoveSaturationID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterColorControlCommandMoveSaturationID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterColorControlCommandStepSaturationID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterColorControlCommandStepSaturationID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterColorControlCommandMoveToHueAndSaturationID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterColorControlCommandMoveToHueAndSaturationID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterColorControlCommandMoveToColorID API_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandMoveToColorID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterColorControlCommandMoveColorID API_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandMoveColorID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterColorControlCommandStepColorID API_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandStepColorID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterColorControlCommandMoveToColorTemperatureID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterColorControlCommandMoveToColorTemperatureID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterColorControlCommandEnhancedMoveToHueID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterColorControlCommandEnhancedMoveToHueID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000040,
    MTRClusterColorControlCommandEnhancedMoveHueID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterColorControlCommandEnhancedMoveHueID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000041,
    MTRClusterColorControlCommandEnhancedStepHueID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterColorControlCommandEnhancedStepHueID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000042,
    MTRClusterColorControlCommandEnhancedMoveToHueAndSaturationID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterColorControlCommandEnhancedMoveToHueAndSaturationID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000043,
    MTRClusterColorControlCommandColorLoopSetID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterColorControlCommandColorLoopSetID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000044,
    MTRClusterColorControlCommandStopMoveStepID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterColorControlCommandStopMoveStepID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000047,
    MTRClusterColorControlCommandMoveColorTemperatureID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterColorControlCommandMoveColorTemperatureID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000004B,
    MTRClusterColorControlCommandStepColorTemperatureID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterColorControlCommandStepColorTemperatureID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000004C,

    // Cluster ColorControl commands
    MTRCommandIDTypeClusterColorControlCommandMoveToHueID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRCommandIDTypeClusterColorControlCommandMoveHueID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRCommandIDTypeClusterColorControlCommandStepHueID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRCommandIDTypeClusterColorControlCommandMoveToSaturationID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRCommandIDTypeClusterColorControlCommandMoveSaturationID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRCommandIDTypeClusterColorControlCommandStepSaturationID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTRCommandIDTypeClusterColorControlCommandMoveToHueAndSaturationID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,
    MTRCommandIDTypeClusterColorControlCommandMoveToColorID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000007,
    MTRCommandIDTypeClusterColorControlCommandMoveColorID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000008,
    MTRCommandIDTypeClusterColorControlCommandStepColorID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000009,
    MTRCommandIDTypeClusterColorControlCommandMoveToColorTemperatureID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000A,
    MTRCommandIDTypeClusterColorControlCommandEnhancedMoveToHueID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000040,
    MTRCommandIDTypeClusterColorControlCommandEnhancedMoveHueID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000041,
    MTRCommandIDTypeClusterColorControlCommandEnhancedStepHueID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000042,
    MTRCommandIDTypeClusterColorControlCommandEnhancedMoveToHueAndSaturationID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000043,
    MTRCommandIDTypeClusterColorControlCommandColorLoopSetID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000044,
    MTRCommandIDTypeClusterColorControlCommandStopMoveStepID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000047,
    MTRCommandIDTypeClusterColorControlCommandMoveColorTemperatureID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000004B,
    MTRCommandIDTypeClusterColorControlCommandStepColorTemperatureID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000004C,

    // Cluster Channel deprecated command id names
    MTRClusterChannelCommandChangeChannelID API_DEPRECATED("Please use MTRCommandIDTypeClusterChannelCommandChangeChannelID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterChannelCommandChangeChannelResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterChannelCommandChangeChannelResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterChannelCommandChangeChannelByNumberID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterChannelCommandChangeChannelByNumberID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterChannelCommandSkipChannelID API_DEPRECATED("Please use MTRCommandIDTypeClusterChannelCommandSkipChannelID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,

    // Cluster Channel commands
    MTRCommandIDTypeClusterChannelCommandChangeChannelID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRCommandIDTypeClusterChannelCommandChangeChannelResponseID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRCommandIDTypeClusterChannelCommandChangeChannelByNumberID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRCommandIDTypeClusterChannelCommandSkipChannelID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,

    // Cluster TargetNavigator deprecated command id names
    MTRClusterTargetNavigatorCommandNavigateTargetID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterTargetNavigatorCommandNavigateTargetID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterTargetNavigatorCommandNavigateTargetResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterTargetNavigatorCommandNavigateTargetResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,

    // Cluster TargetNavigator commands
    MTRCommandIDTypeClusterTargetNavigatorCommandNavigateTargetID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRCommandIDTypeClusterTargetNavigatorCommandNavigateTargetResponseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,

    // Cluster MediaPlayback deprecated command id names
    MTRClusterMediaPlaybackCommandPlayID API_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandPlayID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterMediaPlaybackCommandPauseID API_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandPauseID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterMediaPlaybackCommandStopPlaybackID API_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandStopID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterMediaPlaybackCommandStartOverID API_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandStartOverID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterMediaPlaybackCommandPreviousID API_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandPreviousID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterMediaPlaybackCommandNextID API_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandNextID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterMediaPlaybackCommandRewindID API_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandRewindID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterMediaPlaybackCommandFastForwardID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterMediaPlaybackCommandFastForwardID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterMediaPlaybackCommandSkipForwardID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterMediaPlaybackCommandSkipForwardID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterMediaPlaybackCommandSkipBackwardID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterMediaPlaybackCommandSkipBackwardID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterMediaPlaybackCommandPlaybackResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterMediaPlaybackCommandPlaybackResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterMediaPlaybackCommandSeekID API_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandSeekID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000B,

    // Cluster MediaPlayback commands
    MTRCommandIDTypeClusterMediaPlaybackCommandPlayID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterMediaPlaybackCommandPauseID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterMediaPlaybackCommandStopID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRCommandIDTypeClusterMediaPlaybackCommandStartOverID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRCommandIDTypeClusterMediaPlaybackCommandPreviousID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRCommandIDTypeClusterMediaPlaybackCommandNextID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRCommandIDTypeClusterMediaPlaybackCommandRewindID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,
    MTRCommandIDTypeClusterMediaPlaybackCommandFastForwardID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000007,
    MTRCommandIDTypeClusterMediaPlaybackCommandSkipForwardID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000008,
    MTRCommandIDTypeClusterMediaPlaybackCommandSkipBackwardID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000009,
    MTRCommandIDTypeClusterMediaPlaybackCommandPlaybackResponseID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000A,
    MTRCommandIDTypeClusterMediaPlaybackCommandSeekID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000B,

    // Cluster MediaInput deprecated command id names
    MTRClusterMediaInputCommandSelectInputID API_DEPRECATED("Please use MTRCommandIDTypeClusterMediaInputCommandSelectInputID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterMediaInputCommandShowInputStatusID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterMediaInputCommandShowInputStatusID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterMediaInputCommandHideInputStatusID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterMediaInputCommandHideInputStatusID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterMediaInputCommandRenameInputID API_DEPRECATED("Please use MTRCommandIDTypeClusterMediaInputCommandRenameInputID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,

    // Cluster MediaInput commands
    MTRCommandIDTypeClusterMediaInputCommandSelectInputID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRCommandIDTypeClusterMediaInputCommandShowInputStatusID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRCommandIDTypeClusterMediaInputCommandHideInputStatusID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRCommandIDTypeClusterMediaInputCommandRenameInputID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,

    // Cluster LowPower deprecated command id names
    MTRClusterLowPowerCommandSleepID API_DEPRECATED("Please use MTRCommandIDTypeClusterLowPowerCommandSleepID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,

    // Cluster LowPower commands
    MTRCommandIDTypeClusterLowPowerCommandSleepID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,

    // Cluster KeypadInput deprecated command id names
    MTRClusterKeypadInputCommandSendKeyID API_DEPRECATED("Please use MTRCommandIDTypeClusterKeypadInputCommandSendKeyID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterKeypadInputCommandSendKeyResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterKeypadInputCommandSendKeyResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,

    // Cluster KeypadInput commands
    MTRCommandIDTypeClusterKeypadInputCommandSendKeyID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterKeypadInputCommandSendKeyResponseID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,

    // Cluster ContentLauncher deprecated command id names
    MTRClusterContentLauncherCommandLaunchContentID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterContentLauncherCommandLaunchContentID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterContentLauncherCommandLaunchURLID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterContentLauncherCommandLaunchURLID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterContentLauncherCommandLaunchResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterContentLauncherCommandLauncherResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,

    // Cluster ContentLauncher commands
    MTRCommandIDTypeClusterContentLauncherCommandLaunchContentID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRCommandIDTypeClusterContentLauncherCommandLaunchURLID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRCommandIDTypeClusterContentLauncherCommandLauncherResponseID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,

    // Cluster AudioOutput deprecated command id names
    MTRClusterAudioOutputCommandSelectOutputID API_DEPRECATED("Please use MTRCommandIDTypeClusterAudioOutputCommandSelectOutputID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterAudioOutputCommandRenameOutputID API_DEPRECATED("Please use MTRCommandIDTypeClusterAudioOutputCommandRenameOutputID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,

    // Cluster AudioOutput commands
    MTRCommandIDTypeClusterAudioOutputCommandSelectOutputID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRCommandIDTypeClusterAudioOutputCommandRenameOutputID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,

    // Cluster ApplicationLauncher deprecated command id names
    MTRClusterApplicationLauncherCommandLaunchAppID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterApplicationLauncherCommandLaunchAppID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterApplicationLauncherCommandStopAppID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterApplicationLauncherCommandStopAppID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterApplicationLauncherCommandHideAppID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterApplicationLauncherCommandHideAppID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterApplicationLauncherCommandLauncherResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterApplicationLauncherCommandLauncherResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,

    // Cluster ApplicationLauncher commands
    MTRCommandIDTypeClusterApplicationLauncherCommandLaunchAppID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRCommandIDTypeClusterApplicationLauncherCommandStopAppID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRCommandIDTypeClusterApplicationLauncherCommandHideAppID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRCommandIDTypeClusterApplicationLauncherCommandLauncherResponseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,

    // Cluster AccountLogin deprecated command id names
    MTRClusterAccountLoginCommandGetSetupPINID API_DEPRECATED("Please use MTRCommandIDTypeClusterAccountLoginCommandGetSetupPINID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterAccountLoginCommandGetSetupPINResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterAccountLoginCommandGetSetupPINResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterAccountLoginCommandLoginID API_DEPRECATED("Please use MTRCommandIDTypeClusterAccountLoginCommandLoginID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterAccountLoginCommandLogoutID API_DEPRECATED("Please use MTRCommandIDTypeClusterAccountLoginCommandLogoutID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,

    // Cluster AccountLogin commands
    MTRCommandIDTypeClusterAccountLoginCommandGetSetupPINID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRCommandIDTypeClusterAccountLoginCommandGetSetupPINResponseID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRCommandIDTypeClusterAccountLoginCommandLoginID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRCommandIDTypeClusterAccountLoginCommandLogoutID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,

    // Cluster ElectricalMeasurement deprecated command id names
    MTRClusterElectricalMeasurementCommandGetProfileInfoResponseCommandID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterElectricalMeasurementCommandGetProfileInfoResponseCommandID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterElectricalMeasurementCommandGetProfileInfoCommandID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterElectricalMeasurementCommandGetProfileInfoCommandID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterElectricalMeasurementCommandGetMeasurementProfileResponseCommandID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterElectricalMeasurementCommandGetMeasurementProfileResponseCommandID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterElectricalMeasurementCommandGetMeasurementProfileCommandID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterElectricalMeasurementCommandGetMeasurementProfileCommandID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,

    // Cluster ElectricalMeasurement commands
    MTRCommandIDTypeClusterElectricalMeasurementCommandGetProfileInfoResponseCommandID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRCommandIDTypeClusterElectricalMeasurementCommandGetProfileInfoCommandID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRCommandIDTypeClusterElectricalMeasurementCommandGetMeasurementProfileResponseCommandID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRCommandIDTypeClusterElectricalMeasurementCommandGetMeasurementProfileCommandID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,

    // Cluster TestCluster deprecated command id names
    MTRClusterTestClusterCommandTestID API_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterTestClusterCommandTestSpecificResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestSpecificResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterTestClusterCommandTestNotHandledID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestNotHandledID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterTestClusterCommandTestAddArgumentsResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestAddArgumentsResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterTestClusterCommandTestSpecificID API_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestSpecificID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterTestClusterCommandTestSimpleArgumentResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestSimpleArgumentResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterTestClusterCommandTestUnknownCommandID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestUnknownCommandID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterTestClusterCommandTestStructArrayArgumentResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestStructArrayArgumentResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterTestClusterCommandTestAddArgumentsID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestAddArgumentsID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterTestClusterCommandTestListInt8UReverseResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestListInt8UReverseResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterTestClusterCommandTestSimpleArgumentRequestID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestSimpleArgumentRequestID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterTestClusterCommandTestEnumsResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestEnumsResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterTestClusterCommandTestStructArrayArgumentRequestID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestStructArrayArgumentRequestID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterTestClusterCommandTestNullableOptionalResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestNullableOptionalResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterTestClusterCommandTestStructArgumentRequestID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestStructArgumentRequestID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterTestClusterCommandTestComplexNullableOptionalResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestComplexNullableOptionalResponseID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterTestClusterCommandTestNestedStructArgumentRequestID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestNestedStructArgumentRequestID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterTestClusterCommandBooleanResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandBooleanResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterTestClusterCommandTestListStructArgumentRequestID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestListStructArgumentRequestID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterTestClusterCommandSimpleStructResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandSimpleStructResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterTestClusterCommandTestListInt8UArgumentRequestID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestListInt8UArgumentRequestID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterTestClusterCommandTestEmitTestEventResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestEventResponseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterTestClusterCommandTestNestedStructListArgumentRequestID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestNestedStructListArgumentRequestID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000B,
    MTRClusterTestClusterCommandTestEmitTestFabricScopedEventResponseID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestFabricScopedEventResponseID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000B,
    MTRClusterTestClusterCommandTestListNestedStructListArgumentRequestID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestListNestedStructListArgumentRequestID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000C,
    MTRClusterTestClusterCommandTestListInt8UReverseRequestID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestListInt8UReverseRequestID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000D,
    MTRClusterTestClusterCommandTestEnumsRequestID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestEnumsRequestID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000E,
    MTRClusterTestClusterCommandTestNullableOptionalRequestID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestNullableOptionalRequestID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000F,
    MTRClusterTestClusterCommandTestComplexNullableOptionalRequestID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestComplexNullableOptionalRequestID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,
    MTRClusterTestClusterCommandSimpleStructEchoRequestID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandSimpleStructEchoRequestID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterTestClusterCommandTimedInvokeRequestID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTimedInvokeRequestID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterTestClusterCommandTestSimpleOptionalArgumentRequestID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestSimpleOptionalArgumentRequestID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000013,
    MTRClusterTestClusterCommandTestEmitTestEventRequestID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestEventRequestID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000014,
    MTRClusterTestClusterCommandTestEmitTestFabricScopedEventRequestID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestFabricScopedEventRequestID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000015,

    // Cluster UnitTesting commands
    MTRCommandIDTypeClusterUnitTestingCommandTestID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterUnitTestingCommandTestSpecificResponseID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRCommandIDTypeClusterUnitTestingCommandTestNotHandledID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRCommandIDTypeClusterUnitTestingCommandTestAddArgumentsResponseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTRCommandIDTypeClusterUnitTestingCommandTestSpecificID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRCommandIDTypeClusterUnitTestingCommandTestSimpleArgumentResponseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTRCommandIDTypeClusterUnitTestingCommandTestUnknownCommandID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRCommandIDTypeClusterUnitTestingCommandTestStructArrayArgumentResponseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTRCommandIDTypeClusterUnitTestingCommandTestAddArgumentsID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRCommandIDTypeClusterUnitTestingCommandTestListInt8UReverseResponseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTRCommandIDTypeClusterUnitTestingCommandTestSimpleArgumentRequestID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTRCommandIDTypeClusterUnitTestingCommandTestEnumsResponseID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTRCommandIDTypeClusterUnitTestingCommandTestStructArrayArgumentRequestID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,
    MTRCommandIDTypeClusterUnitTestingCommandTestNullableOptionalResponseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,
    MTRCommandIDTypeClusterUnitTestingCommandTestStructArgumentRequestID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000007,
    MTRCommandIDTypeClusterUnitTestingCommandTestComplexNullableOptionalResponseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000007,
    MTRCommandIDTypeClusterUnitTestingCommandTestNestedStructArgumentRequestID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000008,
    MTRCommandIDTypeClusterUnitTestingCommandBooleanResponseID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000008,
    MTRCommandIDTypeClusterUnitTestingCommandTestListStructArgumentRequestID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000009,
    MTRCommandIDTypeClusterUnitTestingCommandSimpleStructResponseID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000009,
    MTRCommandIDTypeClusterUnitTestingCommandTestListInt8UArgumentRequestID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000A,
    MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestEventResponseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000A,
    MTRCommandIDTypeClusterUnitTestingCommandTestNestedStructListArgumentRequestID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000B,
    MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestFabricScopedEventResponseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000B,
    MTRCommandIDTypeClusterUnitTestingCommandTestListNestedStructListArgumentRequestID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000C,
    MTRCommandIDTypeClusterUnitTestingCommandTestListInt8UReverseRequestID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000D,
    MTRCommandIDTypeClusterUnitTestingCommandTestEnumsRequestID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000E,
    MTRCommandIDTypeClusterUnitTestingCommandTestNullableOptionalRequestID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000F,
    MTRCommandIDTypeClusterUnitTestingCommandTestComplexNullableOptionalRequestID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000010,
    MTRCommandIDTypeClusterUnitTestingCommandSimpleStructEchoRequestID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000011,
    MTRCommandIDTypeClusterUnitTestingCommandTimedInvokeRequestID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000012,
    MTRCommandIDTypeClusterUnitTestingCommandTestSimpleOptionalArgumentRequestID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000013,
    MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestEventRequestID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000014,
    MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestFabricScopedEventRequestID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000015,

    // Cluster FaultInjection deprecated command id names
    MTRClusterFaultInjectionCommandFailAtFaultID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterFaultInjectionCommandFailAtFaultID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterFaultInjectionCommandFailRandomlyAtFaultID API_DEPRECATED(
        "Please use MTRCommandIDTypeClusterFaultInjectionCommandFailRandomlyAtFaultID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,

    // Cluster FaultInjection commands
    MTRCommandIDTypeClusterFaultInjectionCommandFailAtFaultID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTRCommandIDTypeClusterFaultInjectionCommandFailRandomlyAtFaultID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,

};

#pragma mark - Events IDs

typedef NS_ENUM(uint32_t, MTREventIDType) {

    // Cluster AccessControl deprecated event names
    MTRClusterAccessControlEventAccessControlEntryChangedID API_DEPRECATED(
        "Please use MTREventIDTypeClusterAccessControlEventAccessControlEntryChangedID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterAccessControlEventAccessControlExtensionChangedID API_DEPRECATED(
        "Please use MTREventIDTypeClusterAccessControlEventAccessControlExtensionChangedID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,

    // Cluster AccessControl events
    MTREventIDTypeClusterAccessControlEventAccessControlEntryChangedID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTREventIDTypeClusterAccessControlEventAccessControlExtensionChangedID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,

    // Cluster Actions deprecated event names
    MTRClusterActionsEventStateChangedID API_DEPRECATED("Please use MTREventIDTypeClusterActionsEventStateChangedID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterActionsEventActionFailedID API_DEPRECATED("Please use MTREventIDTypeClusterActionsEventActionFailedID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,

    // Cluster Actions events
    MTREventIDTypeClusterActionsEventStateChangedID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTREventIDTypeClusterActionsEventActionFailedID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,

    // Cluster Basic deprecated event names
    MTRClusterBasicEventStartUpID API_DEPRECATED("Please use MTREventIDTypeClusterBasicInformationEventStartUpID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterBasicEventShutDownID API_DEPRECATED("Please use MTREventIDTypeClusterBasicInformationEventShutDownID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterBasicEventLeaveID API_DEPRECATED("Please use MTREventIDTypeClusterBasicInformationEventLeaveID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterBasicEventReachableChangedID API_DEPRECATED("Please use MTREventIDTypeClusterBasicInformationEventReachableChangedID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,

    // Cluster BasicInformation events
    MTREventIDTypeClusterBasicInformationEventStartUpID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTREventIDTypeClusterBasicInformationEventShutDownID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTREventIDTypeClusterBasicInformationEventLeaveID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTREventIDTypeClusterBasicInformationEventReachableChangedID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,

    // Cluster OtaSoftwareUpdateRequestor deprecated event names
    MTRClusterOtaSoftwareUpdateRequestorEventStateTransitionID API_DEPRECATED(
        "Please use MTREventIDTypeClusterOTASoftwareUpdateRequestorEventStateTransitionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterOtaSoftwareUpdateRequestorEventVersionAppliedID API_DEPRECATED(
        "Please use MTREventIDTypeClusterOTASoftwareUpdateRequestorEventVersionAppliedID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterOtaSoftwareUpdateRequestorEventDownloadErrorID API_DEPRECATED(
        "Please use MTREventIDTypeClusterOTASoftwareUpdateRequestorEventDownloadErrorID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,

    // Cluster OTASoftwareUpdateRequestor events
    MTREventIDTypeClusterOTASoftwareUpdateRequestorEventStateTransitionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTREventIDTypeClusterOTASoftwareUpdateRequestorEventVersionAppliedID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTREventIDTypeClusterOTASoftwareUpdateRequestorEventDownloadErrorID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,

    // Cluster PowerSource deprecated event names

    // Cluster PowerSource events
    MTREventIDTypeClusterPowerSourceEventWiredFaultChangeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTREventIDTypeClusterPowerSourceEventBatFaultChangeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTREventIDTypeClusterPowerSourceEventBatChargeFaultChangeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,

    // Cluster GeneralDiagnostics deprecated event names
    MTRClusterGeneralDiagnosticsEventHardwareFaultChangeID API_DEPRECATED(
        "Please use MTREventIDTypeClusterGeneralDiagnosticsEventHardwareFaultChangeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterGeneralDiagnosticsEventRadioFaultChangeID API_DEPRECATED(
        "Please use MTREventIDTypeClusterGeneralDiagnosticsEventRadioFaultChangeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterGeneralDiagnosticsEventNetworkFaultChangeID API_DEPRECATED(
        "Please use MTREventIDTypeClusterGeneralDiagnosticsEventNetworkFaultChangeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterGeneralDiagnosticsEventBootReasonID API_DEPRECATED(
        "Please use MTREventIDTypeClusterGeneralDiagnosticsEventBootReasonID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,

    // Cluster GeneralDiagnostics events
    MTREventIDTypeClusterGeneralDiagnosticsEventHardwareFaultChangeID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTREventIDTypeClusterGeneralDiagnosticsEventRadioFaultChangeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTREventIDTypeClusterGeneralDiagnosticsEventNetworkFaultChangeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTREventIDTypeClusterGeneralDiagnosticsEventBootReasonID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,

    // Cluster SoftwareDiagnostics deprecated event names
    MTRClusterSoftwareDiagnosticsEventSoftwareFaultID API_DEPRECATED(
        "Please use MTREventIDTypeClusterSoftwareDiagnosticsEventSoftwareFaultID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,

    // Cluster SoftwareDiagnostics events
    MTREventIDTypeClusterSoftwareDiagnosticsEventSoftwareFaultID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,

    // Cluster ThreadNetworkDiagnostics deprecated event names
    MTRClusterThreadNetworkDiagnosticsEventConnectionStatusID API_DEPRECATED(
        "Please use MTREventIDTypeClusterThreadNetworkDiagnosticsEventConnectionStatusID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterThreadNetworkDiagnosticsEventNetworkFaultChangeID API_DEPRECATED(
        "Please use MTREventIDTypeClusterThreadNetworkDiagnosticsEventNetworkFaultChangeID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,

    // Cluster ThreadNetworkDiagnostics events
    MTREventIDTypeClusterThreadNetworkDiagnosticsEventConnectionStatusID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTREventIDTypeClusterThreadNetworkDiagnosticsEventNetworkFaultChangeID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,

    // Cluster WiFiNetworkDiagnostics deprecated event names
    MTRClusterWiFiNetworkDiagnosticsEventDisconnectionID API_DEPRECATED(
        "Please use MTREventIDTypeClusterWiFiNetworkDiagnosticsEventDisconnectionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterWiFiNetworkDiagnosticsEventAssociationFailureID API_DEPRECATED(
        "Please use MTREventIDTypeClusterWiFiNetworkDiagnosticsEventAssociationFailureID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterWiFiNetworkDiagnosticsEventConnectionStatusID API_DEPRECATED(
        "Please use MTREventIDTypeClusterWiFiNetworkDiagnosticsEventConnectionStatusID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,

    // Cluster WiFiNetworkDiagnostics events
    MTREventIDTypeClusterWiFiNetworkDiagnosticsEventDisconnectionID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTREventIDTypeClusterWiFiNetworkDiagnosticsEventAssociationFailureID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTREventIDTypeClusterWiFiNetworkDiagnosticsEventConnectionStatusID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,

    // Cluster BridgedDeviceBasic deprecated event names
    MTRClusterBridgedDeviceBasicEventStartUpID API_DEPRECATED(
        "Please use MTREventIDTypeClusterBridgedDeviceBasicInformationEventStartUpID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterBridgedDeviceBasicEventShutDownID API_DEPRECATED(
        "Please use MTREventIDTypeClusterBridgedDeviceBasicInformationEventShutDownID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterBridgedDeviceBasicEventLeaveID API_DEPRECATED(
        "Please use MTREventIDTypeClusterBridgedDeviceBasicInformationEventLeaveID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterBridgedDeviceBasicEventReachableChangedID API_DEPRECATED(
        "Please use MTREventIDTypeClusterBridgedDeviceBasicInformationEventReachableChangedID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,

    // Cluster BridgedDeviceBasicInformation events
    MTREventIDTypeClusterBridgedDeviceBasicInformationEventStartUpID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTREventIDTypeClusterBridgedDeviceBasicInformationEventShutDownID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTREventIDTypeClusterBridgedDeviceBasicInformationEventLeaveID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTREventIDTypeClusterBridgedDeviceBasicInformationEventReachableChangedID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,

    // Cluster Switch deprecated event names
    MTRClusterSwitchEventSwitchLatchedID API_DEPRECATED("Please use MTREventIDTypeClusterSwitchEventSwitchLatchedID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterSwitchEventInitialPressID API_DEPRECATED("Please use MTREventIDTypeClusterSwitchEventInitialPressID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterSwitchEventLongPressID API_DEPRECATED("Please use MTREventIDTypeClusterSwitchEventLongPressID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterSwitchEventShortReleaseID API_DEPRECATED("Please use MTREventIDTypeClusterSwitchEventShortReleaseID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterSwitchEventLongReleaseID API_DEPRECATED("Please use MTREventIDTypeClusterSwitchEventLongReleaseID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterSwitchEventMultiPressOngoingID API_DEPRECATED("Please use MTREventIDTypeClusterSwitchEventMultiPressOngoingID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterSwitchEventMultiPressCompleteID API_DEPRECATED("Please use MTREventIDTypeClusterSwitchEventMultiPressCompleteID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,

    // Cluster Switch events
    MTREventIDTypeClusterSwitchEventSwitchLatchedID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTREventIDTypeClusterSwitchEventInitialPressID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTREventIDTypeClusterSwitchEventLongPressID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTREventIDTypeClusterSwitchEventShortReleaseID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTREventIDTypeClusterSwitchEventLongReleaseID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTREventIDTypeClusterSwitchEventMultiPressOngoingID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTREventIDTypeClusterSwitchEventMultiPressCompleteID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,

    // Cluster BooleanState deprecated event names
    MTRClusterBooleanStateEventStateChangeID API_DEPRECATED("Please use MTREventIDTypeClusterBooleanStateEventStateChangeID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,

    // Cluster BooleanState events
    MTREventIDTypeClusterBooleanStateEventStateChangeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,

    // Cluster DoorLock deprecated event names
    MTRClusterDoorLockEventDoorLockAlarmID API_DEPRECATED("Please use MTREventIDTypeClusterDoorLockEventDoorLockAlarmID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterDoorLockEventDoorStateChangeID API_DEPRECATED("Please use MTREventIDTypeClusterDoorLockEventDoorStateChangeID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterDoorLockEventLockOperationID API_DEPRECATED("Please use MTREventIDTypeClusterDoorLockEventLockOperationID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterDoorLockEventLockOperationErrorID API_DEPRECATED("Please use MTREventIDTypeClusterDoorLockEventLockOperationErrorID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterDoorLockEventLockUserChangeID API_DEPRECATED("Please use MTREventIDTypeClusterDoorLockEventLockUserChangeID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,

    // Cluster DoorLock events
    MTREventIDTypeClusterDoorLockEventDoorLockAlarmID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTREventIDTypeClusterDoorLockEventDoorStateChangeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTREventIDTypeClusterDoorLockEventLockOperationID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTREventIDTypeClusterDoorLockEventLockOperationErrorID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTREventIDTypeClusterDoorLockEventLockUserChangeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,

    // Cluster PumpConfigurationAndControl deprecated event names
    MTRClusterPumpConfigurationAndControlEventSupplyVoltageLowID API_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventSupplyVoltageLowID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterPumpConfigurationAndControlEventSupplyVoltageHighID API_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventSupplyVoltageHighID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterPumpConfigurationAndControlEventPowerMissingPhaseID API_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventPowerMissingPhaseID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterPumpConfigurationAndControlEventSystemPressureLowID API_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventSystemPressureLowID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterPumpConfigurationAndControlEventSystemPressureHighID API_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventSystemPressureHighID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterPumpConfigurationAndControlEventDryRunningID API_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventDryRunningID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterPumpConfigurationAndControlEventMotorTemperatureHighID API_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventMotorTemperatureHighID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterPumpConfigurationAndControlEventPumpMotorFatalFailureID API_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventPumpMotorFatalFailureID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterPumpConfigurationAndControlEventElectronicTemperatureHighID API_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventElectronicTemperatureHighID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterPumpConfigurationAndControlEventPumpBlockedID API_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventPumpBlockedID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterPumpConfigurationAndControlEventSensorFailureID API_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventSensorFailureID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterPumpConfigurationAndControlEventElectronicNonFatalFailureID API_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventElectronicNonFatalFailureID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000B,
    MTRClusterPumpConfigurationAndControlEventElectronicFatalFailureID API_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventElectronicFatalFailureID", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000C,
    MTRClusterPumpConfigurationAndControlEventGeneralFaultID API_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventGeneralFaultID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000D,
    MTRClusterPumpConfigurationAndControlEventLeakageID API_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventLeakageID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000E,
    MTRClusterPumpConfigurationAndControlEventAirDetectionID API_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventAirDetectionID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000F,
    MTRClusterPumpConfigurationAndControlEventTurbineOperationID API_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventTurbineOperationID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,

    // Cluster PumpConfigurationAndControl events
    MTREventIDTypeClusterPumpConfigurationAndControlEventSupplyVoltageLowID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000000,
    MTREventIDTypeClusterPumpConfigurationAndControlEventSupplyVoltageHighID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000001,
    MTREventIDTypeClusterPumpConfigurationAndControlEventPowerMissingPhaseID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,
    MTREventIDTypeClusterPumpConfigurationAndControlEventSystemPressureLowID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000003,
    MTREventIDTypeClusterPumpConfigurationAndControlEventSystemPressureHighID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000004,
    MTREventIDTypeClusterPumpConfigurationAndControlEventDryRunningID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000005,
    MTREventIDTypeClusterPumpConfigurationAndControlEventMotorTemperatureHighID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000006,
    MTREventIDTypeClusterPumpConfigurationAndControlEventPumpMotorFatalFailureID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000007,
    MTREventIDTypeClusterPumpConfigurationAndControlEventElectronicTemperatureHighID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000008,
    MTREventIDTypeClusterPumpConfigurationAndControlEventPumpBlockedID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000009,
    MTREventIDTypeClusterPumpConfigurationAndControlEventSensorFailureID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000A,
    MTREventIDTypeClusterPumpConfigurationAndControlEventElectronicNonFatalFailureID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000B,
    MTREventIDTypeClusterPumpConfigurationAndControlEventElectronicFatalFailureID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000C,
    MTREventIDTypeClusterPumpConfigurationAndControlEventGeneralFaultID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000D,
    MTREventIDTypeClusterPumpConfigurationAndControlEventLeakageID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000E,
    MTREventIDTypeClusterPumpConfigurationAndControlEventAirDetectionID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x0000000F,
    MTREventIDTypeClusterPumpConfigurationAndControlEventTurbineOperationID API_AVAILABLE(
        ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000010,

    // Cluster TestCluster deprecated event names
    MTRClusterTestClusterEventTestEventID API_DEPRECATED("Please use MTREventIDTypeClusterUnitTestingEventTestEventID",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterTestClusterEventTestFabricScopedEventID API_DEPRECATED(
        "Please use MTREventIDTypeClusterUnitTestingEventTestFabricScopedEventID", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,

    // Cluster UnitTesting events
    MTREventIDTypeClusterUnitTestingEventTestEventID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTREventIDTypeClusterUnitTestingEventTestFabricScopedEventID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    = 0x00000002,

};
