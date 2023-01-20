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

NS_ASSUME_NONNULL_BEGIN

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterAttributeValuePair : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable attributeId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSArray * _Nonnull attributeValue API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterExtensionFieldSet : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull clusterId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSArray * _Nonnull attributeValueList API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.2), macos(13.1), watchos(9.2), tvos(16.2))
@interface MTRDescriptorClusterDeviceTypeStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull deviceType MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull type API_AVAILABLE(ios(16.2), macos(13.1), watchos(9.2), tvos(16.2))
    MTR_NEWLY_DEPRECATED("Please use deviceType");
@property (nonatomic, copy) NSNumber * _Nonnull revision API_AVAILABLE(ios(16.2), macos(13.1), watchos(9.2), tvos(16.2));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRDescriptorClusterDeviceTypeStruct")
@interface MTRDescriptorClusterDeviceType : MTRDescriptorClusterDeviceTypeStruct
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRBindingClusterTargetStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable node API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable group API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable endpoint API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable cluster API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRAccessControlClusterTarget : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable cluster API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable endpoint API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable deviceType API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_NEWLY_AVAILABLE
@interface MTRAccessControlClusterAccessControlEntryStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull privilege MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull authMode MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nullable subjects MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nullable targets MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRAccessControlClusterAccessControlEntryStruct")
@interface MTRAccessControlClusterAccessControlEntry : MTRAccessControlClusterAccessControlEntryStruct
@end
MTR_NEWLY_AVAILABLE
@interface MTRAccessControlClusterAccessControlExtensionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSData * _Nonnull data MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRAccessControlClusterAccessControlExtensionStruct")
@interface MTRAccessControlClusterExtensionEntry : MTRAccessControlClusterAccessControlExtensionStruct
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRAccessControlClusterAccessControlEntryChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable adminNodeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable adminPasscodeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull changeType API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) MTRAccessControlClusterAccessControlEntryStruct * _Nullable latestValue API_AVAILABLE(
    ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRAccessControlClusterAccessControlExtensionChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable adminNodeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable adminPasscodeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull changeType API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) MTRAccessControlClusterAccessControlExtensionStruct * _Nullable latestValue API_AVAILABLE(
    ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterActionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull actionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSString * _Nonnull name API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull type API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull endpointListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull supportedCommands API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull state API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterEndpointListStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull endpointListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSString * _Nonnull name API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull type API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSArray * _Nonnull endpoints API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterStateChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull actionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull invokeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy, getter=getNewState)
    NSNumber * _Nonnull newState API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterActionFailedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull actionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull invokeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy, getter=getNewState)
    NSNumber * _Nonnull newState API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull error API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_NEWLY_AVAILABLE
@interface MTRBasicInformationClusterCapabilityMinimaStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull caseSessionsPerFabric MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull subscriptionsPerFabric MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRBasicInformationClusterCapabilityMinimaStruct")
@interface MTRBasicClusterCapabilityMinimaStruct : MTRBasicInformationClusterCapabilityMinimaStruct
@end

MTR_NEWLY_AVAILABLE
@interface MTRBasicInformationClusterStartUpEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRBasicInformationClusterStartUpEvent")
@interface MTRBasicClusterStartUpEvent : MTRBasicInformationClusterStartUpEvent
@end

MTR_NEWLY_AVAILABLE
@interface MTRBasicInformationClusterShutDownEvent : NSObject <NSCopying>
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRBasicInformationClusterShutDownEvent")
@interface MTRBasicClusterShutDownEvent : MTRBasicInformationClusterShutDownEvent
@end

MTR_NEWLY_AVAILABLE
@interface MTRBasicInformationClusterLeaveEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRBasicInformationClusterLeaveEvent")
@interface MTRBasicClusterLeaveEvent : MTRBasicInformationClusterLeaveEvent
@end

MTR_NEWLY_AVAILABLE
@interface MTRBasicInformationClusterReachableChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull reachableNewValue MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRBasicInformationClusterReachableChangedEvent")
@interface MTRBasicClusterReachableChangedEvent : MTRBasicInformationClusterReachableChangedEvent
@end

MTR_NEWLY_AVAILABLE
@interface MTROTASoftwareUpdateRequestorClusterProviderLocation : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull providerNodeID MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull endpoint MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterProviderLocation")
@interface MTROtaSoftwareUpdateRequestorClusterProviderLocation : MTROTASoftwareUpdateRequestorClusterProviderLocation
@end

MTR_NEWLY_AVAILABLE
@interface MTROTASoftwareUpdateRequestorClusterStateTransitionEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull previousState MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy, getter=getNewState) NSNumber * _Nonnull newState MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull reason MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable targetSoftwareVersion MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterStateTransitionEvent")
@interface MTROtaSoftwareUpdateRequestorClusterStateTransitionEvent : MTROTASoftwareUpdateRequestorClusterStateTransitionEvent
@end

MTR_NEWLY_AVAILABLE
@interface MTROTASoftwareUpdateRequestorClusterVersionAppliedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull productID MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterVersionAppliedEvent")
@interface MTROtaSoftwareUpdateRequestorClusterVersionAppliedEvent : MTROTASoftwareUpdateRequestorClusterVersionAppliedEvent
@end

MTR_NEWLY_AVAILABLE
@interface MTROTASoftwareUpdateRequestorClusterDownloadErrorEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull bytesDownloaded MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable progressPercent MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable platformCode MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterDownloadErrorEvent")
@interface MTROtaSoftwareUpdateRequestorClusterDownloadErrorEvent : MTROTASoftwareUpdateRequestorClusterDownloadErrorEvent
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("This struct is unused and will be removed")
@interface MTRPowerSourceClusterBatChargeFaultChangeType : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    MTR_NEWLY_DEPRECATED("Please use MTRPowerSourceClusterBatChargeFaultChangeType");
@property (nonatomic, copy) NSArray * _Nonnull previous API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    MTR_NEWLY_DEPRECATED("Please use MTRPowerSourceClusterBatChargeFaultChangeType");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("This struct is unused and will be removed")
@interface MTRPowerSourceClusterBatFaultChangeType : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    MTR_NEWLY_DEPRECATED("Please use MTRPowerSourceClusterBatFaultChangeType");
@property (nonatomic, copy) NSArray * _Nonnull previous API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    MTR_NEWLY_DEPRECATED("Please use MTRPowerSourceClusterBatFaultChangeType");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("This struct is unused and will be removed")
@interface MTRPowerSourceClusterWiredFaultChangeType : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    MTR_NEWLY_DEPRECATED("Please use MTRPowerSourceClusterWiredFaultChangeType");
@property (nonatomic, copy) NSArray * _Nonnull previous API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    MTR_NEWLY_DEPRECATED("Please use MTRPowerSourceClusterWiredFaultChangeType");
@end

MTR_NEWLY_AVAILABLE
@interface MTRPowerSourceClusterWiredFaultChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nonnull previous MTR_NEWLY_AVAILABLE;
@end

MTR_NEWLY_AVAILABLE
@interface MTRPowerSourceClusterBatFaultChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nonnull previous MTR_NEWLY_AVAILABLE;
@end

MTR_NEWLY_AVAILABLE
@interface MTRPowerSourceClusterBatChargeFaultChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nonnull previous MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGeneralCommissioningClusterBasicCommissioningInfo : NSObject <NSCopying>
@property (nonatomic, copy)
    NSNumber * _Nonnull failSafeExpiryLengthSeconds API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy)
    NSNumber * _Nonnull maxCumulativeFailsafeSeconds API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRNetworkCommissioningClusterNetworkInfo : NSObject <NSCopying>
@property (nonatomic, copy) NSData * _Nonnull networkID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull connected API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRNetworkCommissioningClusterThreadInterfaceScanResult : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull panId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull extendedPanId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSString * _Nonnull networkName API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull channel API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull version API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSData * _Nonnull extendedAddress API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull rssi API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull lqi API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRNetworkCommissioningClusterWiFiInterfaceScanResult : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull security API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSData * _Nonnull ssid API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSData * _Nonnull bssid API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull channel API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull wiFiBand API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull rssi API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_NEWLY_AVAILABLE
@interface MTRGeneralDiagnosticsClusterNetworkInterface : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull name MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull isOperational MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable offPremiseServicesReachableIPv4 MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable offPremiseServicesReachableIPv6 MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSData * _Nonnull hardwareAddress MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nonnull iPv4Addresses MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nonnull iPv6Addresses MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull type MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRGeneralDiagnosticsClusterNetworkInterface")
@interface MTRGeneralDiagnosticsClusterNetworkInterfaceType : MTRGeneralDiagnosticsClusterNetworkInterface
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGeneralDiagnosticsClusterHardwareFaultChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSArray * _Nonnull previous API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGeneralDiagnosticsClusterRadioFaultChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSArray * _Nonnull previous API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGeneralDiagnosticsClusterNetworkFaultChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSArray * _Nonnull previous API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGeneralDiagnosticsClusterBootReasonEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull bootReason API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_NEWLY_AVAILABLE
@interface MTRSoftwareDiagnosticsClusterThreadMetricsStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull id MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable name MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable stackFreeCurrent MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable stackFreeMinimum MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable stackSize MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRSoftwareDiagnosticsClusterThreadMetricsStruct")
@interface MTRSoftwareDiagnosticsClusterThreadMetrics : MTRSoftwareDiagnosticsClusterThreadMetricsStruct
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRSoftwareDiagnosticsClusterSoftwareFaultEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull id API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSString * _Nullable name API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSData * _Nullable faultRecording API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRThreadNetworkDiagnosticsClusterNeighborTable : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull extAddress API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull age API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull rloc16 API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull linkFrameCounter API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull mleFrameCounter API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull lqi API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable averageRssi API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable lastRssi API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull frameErrorRate API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull messageErrorRate API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull rxOnWhenIdle API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull fullThreadDevice API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull fullNetworkData API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull isChild API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRThreadNetworkDiagnosticsClusterOperationalDatasetComponents : NSObject <NSCopying>
@property (nonatomic, copy)
    NSNumber * _Nonnull activeTimestampPresent API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy)
    NSNumber * _Nonnull pendingTimestampPresent API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull masterKeyPresent API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull networkNamePresent API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull extendedPanIdPresent API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    ;
@property (nonatomic, copy)
    NSNumber * _Nonnull meshLocalPrefixPresent API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull delayPresent API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull panIdPresent API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull channelPresent API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull pskcPresent API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy)
    NSNumber * _Nonnull securityPolicyPresent API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull channelMaskPresent API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRThreadNetworkDiagnosticsClusterRouteTable : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull extAddress API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull rloc16 API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull routerId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull nextHop API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull pathCost API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull lqiIn API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull lqiOut API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull age API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull allocated API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull linkEstablished API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRThreadNetworkDiagnosticsClusterSecurityPolicy : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull rotationTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull flags API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRThreadNetworkDiagnosticsClusterConnectionStatusEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull connectionStatus API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRThreadNetworkDiagnosticsClusterNetworkFaultChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSArray * _Nonnull previous API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRWiFiNetworkDiagnosticsClusterDisconnectionEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull reasonCode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRWiFiNetworkDiagnosticsClusterAssociationFailureEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull associationFailure API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRWiFiNetworkDiagnosticsClusterConnectionStatusEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull connectionStatus API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRTimeSynchronizationClusterDstOffsetType : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull offset API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull validStarting API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull validUntil API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRTimeSynchronizationClusterTimeZoneType : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull offset API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull validAt API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSString * _Nullable name API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_NEWLY_AVAILABLE
@interface MTRBridgedDeviceBasicInformationClusterStartUpEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRBridgedDeviceBasicClusterStartUpEvent : MTRBridgedDeviceBasicInformationClusterStartUpEvent
@end

MTR_NEWLY_AVAILABLE
@interface MTRBridgedDeviceBasicInformationClusterShutDownEvent : NSObject <NSCopying>
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRBridgedDeviceBasicClusterShutDownEvent : MTRBridgedDeviceBasicInformationClusterShutDownEvent
@end

MTR_NEWLY_AVAILABLE
@interface MTRBridgedDeviceBasicInformationClusterLeaveEvent : NSObject <NSCopying>
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRBridgedDeviceBasicClusterLeaveEvent : MTRBridgedDeviceBasicInformationClusterLeaveEvent
@end

MTR_NEWLY_AVAILABLE
@interface MTRBridgedDeviceBasicInformationClusterReachableChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull reachableNewValue MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRBridgedDeviceBasicClusterReachableChangedEvent : MTRBridgedDeviceBasicInformationClusterReachableChangedEvent
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRSwitchClusterSwitchLatchedEvent : NSObject <NSCopying>
@property (nonatomic, copy, getter=getNewPosition)
    NSNumber * _Nonnull newPosition API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRSwitchClusterInitialPressEvent : NSObject <NSCopying>
@property (nonatomic, copy, getter=getNewPosition)
    NSNumber * _Nonnull newPosition API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRSwitchClusterLongPressEvent : NSObject <NSCopying>
@property (nonatomic, copy, getter=getNewPosition)
    NSNumber * _Nonnull newPosition API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRSwitchClusterShortReleaseEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull previousPosition API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRSwitchClusterLongReleaseEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull previousPosition API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRSwitchClusterMultiPressOngoingEvent : NSObject <NSCopying>
@property (nonatomic, copy, getter=getNewPosition)
    NSNumber * _Nonnull newPosition API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy)
    NSNumber * _Nonnull currentNumberOfPressesCounted API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRSwitchClusterMultiPressCompleteEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull previousPosition MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy, getter=getNewPosition) NSNumber * _Nonnull newPosition API_AVAILABLE(
    ios(16.1), macos(13.0), watchos(9.1), tvos(16.1)) MTR_NEWLY_DEPRECATED("Please use previousPosition");
@property (nonatomic, copy)
    NSNumber * _Nonnull totalNumberOfPressesCounted API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_NEWLY_AVAILABLE
@interface MTROperationalCredentialsClusterFabricDescriptorStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSData * _Nonnull rootPublicKey MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull vendorID MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull vendorId MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull fabricID MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull fabricId MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull nodeID MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull nodeId MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nonnull label MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTROperationalCredentialsClusterFabricDescriptorStruct")
@interface MTROperationalCredentialsClusterFabricDescriptor : MTROperationalCredentialsClusterFabricDescriptorStruct
@end
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROperationalCredentialsClusterNOCStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSData * _Nonnull noc API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSData * _Nullable icac API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupKeyManagementClusterGroupInfoMapStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull groupId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSArray * _Nonnull endpoints API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSString * _Nullable groupName API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupKeyManagementClusterGroupKeyMapStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull groupId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull groupKeySetID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupKeyManagementClusterGroupKeySetStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull groupKeySetID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy)
    NSNumber * _Nonnull groupKeySecurityPolicy API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSData * _Nullable epochKey0 API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable epochStartTime0 API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSData * _Nullable epochKey1 API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable epochStartTime1 API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSData * _Nullable epochKey2 API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable epochStartTime2 API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRFixedLabelClusterLabelStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull label API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSString * _Nonnull value API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRUserLabelClusterLabelStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull label API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSString * _Nonnull value API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRBooleanStateClusterStateChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull stateValue API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_NEWLY_AVAILABLE
@interface MTRModeSelectClusterSemanticTagStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull mfgCode MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull value MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRModeSelectClusterSemanticTagStruct")
@interface MTRModeSelectClusterSemanticTag : MTRModeSelectClusterSemanticTagStruct
@end
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRModeSelectClusterModeOptionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull label API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull mode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSArray * _Nonnull semanticTags API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_NEWLY_AVAILABLE
@interface MTRDoorLockClusterCredentialStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull credentialType MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull credentialIndex MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRDoorLockClusterCredentialStruct")
@interface MTRDoorLockClusterDlCredential : MTRDoorLockClusterCredentialStruct
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterDoorLockAlarmEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull alarmCode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterDoorStateChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull doorState API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterLockOperationEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull lockOperationType API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull operationSource API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable userIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable fabricIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable sourceNode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSArray * _Nullable credentials API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterLockOperationErrorEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull lockOperationType API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull operationSource API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull operationError API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable userIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable fabricIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable sourceNode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSArray * _Nullable credentials API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterLockUserChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull lockDataType API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull dataOperationType API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull operationSource API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable userIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable fabricIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable sourceNode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable dataIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterSupplyVoltageLowEvent : NSObject <NSCopying>
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterSupplyVoltageHighEvent : NSObject <NSCopying>
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterPowerMissingPhaseEvent : NSObject <NSCopying>
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterSystemPressureLowEvent : NSObject <NSCopying>
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterSystemPressureHighEvent : NSObject <NSCopying>
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterDryRunningEvent : NSObject <NSCopying>
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterMotorTemperatureHighEvent : NSObject <NSCopying>
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterPumpMotorFatalFailureEvent : NSObject <NSCopying>
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterElectronicTemperatureHighEvent : NSObject <NSCopying>
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterPumpBlockedEvent : NSObject <NSCopying>
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterSensorFailureEvent : NSObject <NSCopying>
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterElectronicNonFatalFailureEvent : NSObject <NSCopying>
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterElectronicFatalFailureEvent : NSObject <NSCopying>
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterGeneralFaultEvent : NSObject <NSCopying>
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterLeakageEvent : NSObject <NSCopying>
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterAirDetectionEvent : NSObject <NSCopying>
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterTurbineOperationEvent : NSObject <NSCopying>
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRThermostatClusterThermostatScheduleTransition : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull transitionTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable heatSetpoint API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable coolSetpoint API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRChannelClusterChannelInfo : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull majorNumber API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull minorNumber API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSString * _Nullable name API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSString * _Nullable callSign API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSString * _Nullable affiliateCallSign API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRChannelClusterLineupInfo : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull operatorName API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSString * _Nullable lineupName API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSString * _Nullable postalCode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull lineupInfoType API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_NEWLY_AVAILABLE
@interface MTRTargetNavigatorClusterTargetInfoStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull identifier MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nonnull name MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRTargetNavigatorClusterTargetInfoStruct")
@interface MTRTargetNavigatorClusterTargetInfo : MTRTargetNavigatorClusterTargetInfoStruct
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRMediaPlaybackClusterPlaybackPosition : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull updatedAt API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable position API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRMediaInputClusterInputInfo : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull index API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull inputType API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSString * _Nonnull name API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSString * _Nonnull descriptionString API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRContentLauncherClusterDimension : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull width API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull height API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull metric API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRContentLauncherClusterAdditionalInfo : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull name API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSString * _Nonnull value API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRContentLauncherClusterParameter : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull type API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSString * _Nonnull value API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSArray * _Nullable externalIDList API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRContentLauncherClusterContentSearch : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull parameterList API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRContentLauncherClusterStyleInformation : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nullable imageUrl API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSString * _Nullable color API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy)
    MTRContentLauncherClusterDimension * _Nullable size API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRContentLauncherClusterBrandingInformation : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull providerName API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) MTRContentLauncherClusterStyleInformation * _Nullable background API_AVAILABLE(
    ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy)
    MTRContentLauncherClusterStyleInformation * _Nullable logo API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) MTRContentLauncherClusterStyleInformation * _Nullable progressBar API_AVAILABLE(
    ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy)
    MTRContentLauncherClusterStyleInformation * _Nullable splash API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy)
    MTRContentLauncherClusterStyleInformation * _Nullable waterMark API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRAudioOutputClusterOutputInfo : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull index API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull outputType API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSString * _Nonnull name API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRApplicationLauncherClusterApplication : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull catalogVendorId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSString * _Nonnull applicationId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRApplicationLauncherClusterApplicationEP : NSObject <NSCopying>
@property (nonatomic, copy)
    MTRApplicationLauncherClusterApplication * _Nonnull application API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable endpoint API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRApplicationBasicClusterApplicationBasicApplication : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull catalogVendorId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSString * _Nonnull applicationId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterSimpleStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull a MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull b MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull c MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSData * _Nonnull d MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nonnull e MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull f MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull g MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull h MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterSimpleStruct")
@interface MTRTestClusterClusterSimpleStruct : MTRUnitTestingClusterSimpleStruct
@end
MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestFabricScoped : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull fabricSensitiveInt8u MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable optionalFabricSensitiveInt8u MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable nullableFabricSensitiveInt8u MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalFabricSensitiveInt8u MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nonnull fabricSensitiveCharString MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nonnull fabricSensitiveStruct MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nonnull fabricSensitiveInt8uList MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestFabricScoped")
@interface MTRTestClusterClusterTestFabricScoped : MTRUnitTestingClusterTestFabricScoped
@end
MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterNullablesAndOptionalsStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable nullableInt MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable optionalInt MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalInt MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable nullableString MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable optionalString MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable nullableOptionalString MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable nullableStruct MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable optionalStruct MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable nullableOptionalStruct MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nullable nullableList MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nullable optionalList MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nullable nullableOptionalList MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterNullablesAndOptionalsStruct")
@interface MTRTestClusterClusterNullablesAndOptionalsStruct : MTRUnitTestingClusterNullablesAndOptionalsStruct
@end
MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterNestedStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull a MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull b MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nonnull c MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterNestedStruct")
@interface MTRTestClusterClusterNestedStruct : MTRUnitTestingClusterNestedStruct
@end
MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterNestedStructList : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull a MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull b MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nonnull c MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nonnull d MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nonnull e MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nonnull f MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nonnull g MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterNestedStructList")
@interface MTRTestClusterClusterNestedStructList : MTRUnitTestingClusterNestedStructList
@end
MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterDoubleNestedStructList : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull a MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterDoubleNestedStructList")
@interface MTRTestClusterClusterDoubleNestedStructList : MTRUnitTestingClusterDoubleNestedStructList
@end
MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestListStructOctet : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull member1 MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSData * _Nonnull member2 MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestListStructOctet")
@interface MTRTestClusterClusterTestListStructOctet : MTRUnitTestingClusterTestListStructOctet
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestEventEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull arg1 MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull arg2 MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull arg3 MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nonnull arg4 MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nonnull arg5 MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nonnull arg6 MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestEventEvent")
@interface MTRTestClusterClusterTestEventEvent : MTRUnitTestingClusterTestEventEvent
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestFabricScopedEventEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestFabricScopedEventEvent")
@interface MTRTestClusterClusterTestFabricScopedEventEvent : MTRUnitTestingClusterTestFabricScopedEventEvent
@end

NS_ASSUME_NONNULL_END
