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

NS_ASSUME_NONNULL_BEGIN

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterAttributeValuePair : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable attributeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nullable attributeId MTR_DEPRECATED(
    "Please use attributeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull attributeValue API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterExtensionFieldSet : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull clusterID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull clusterId MTR_DEPRECATED(
    "Please use clusterID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSArray * _Nonnull attributeValueList API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.2), macos(13.1), watchos(9.2), tvos(16.2))
@interface MTRDescriptorClusterDeviceTypeStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull deviceType API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull type MTR_DEPRECATED(
    "Please use deviceType", ios(16.2, 16.4), macos(13.1, 13.3), watchos(9.2, 9.4), tvos(16.2, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull revision API_AVAILABLE(ios(16.2), macos(13.1), watchos(9.2), tvos(16.2));
@end

MTR_DEPRECATED(
    "Please use MTRDescriptorClusterDeviceTypeStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
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

MTR_NEWLY_AVAILABLE
@interface MTRAccessControlClusterAccessControlTargetStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable cluster MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable endpoint MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable deviceType MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRAccessControlClusterAccessControlTargetStruct")
@interface MTRAccessControlClusterTarget : MTRAccessControlClusterAccessControlTargetStruct
@end
API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRAccessControlClusterAccessControlEntryStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull privilege API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull authMode API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nullable subjects API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nullable targets API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRAccessControlClusterAccessControlEntryStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4),
    tvos(16.1, 16.4))
@interface MTRAccessControlClusterAccessControlEntry : MTRAccessControlClusterAccessControlEntryStruct
@end
API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRAccessControlClusterAccessControlExtensionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSData * _Nonnull data API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRAccessControlClusterAccessControlExtensionStruct", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
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

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRBasicInformationClusterCapabilityMinimaStruct : NSObject <NSCopying>
@property (nonatomic, copy)
    NSNumber * _Nonnull caseSessionsPerFabric API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy)
    NSNumber * _Nonnull subscriptionsPerFabric API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRBasicInformationClusterCapabilityMinimaStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4),
    tvos(16.1, 16.4))
@interface MTRBasicClusterCapabilityMinimaStruct : MTRBasicInformationClusterCapabilityMinimaStruct
@end
MTR_NEWLY_AVAILABLE
@interface MTRBasicInformationClusterProductAppearanceStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull finish MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable primaryColor MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRBasicInformationClusterStartUpEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED(
    "Please use MTRBasicInformationClusterStartUpEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRBasicClusterStartUpEvent : MTRBasicInformationClusterStartUpEvent
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRBasicInformationClusterShutDownEvent : NSObject <NSCopying>
@end

MTR_DEPRECATED(
    "Please use MTRBasicInformationClusterShutDownEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRBasicClusterShutDownEvent : MTRBasicInformationClusterShutDownEvent
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRBasicInformationClusterLeaveEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED(
    "Please use MTRBasicInformationClusterLeaveEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRBasicClusterLeaveEvent : MTRBasicInformationClusterLeaveEvent
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRBasicInformationClusterReachableChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull reachableNewValue API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRBasicInformationClusterReachableChangedEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4),
    tvos(16.1, 16.4))
@interface MTRBasicClusterReachableChangedEvent : MTRBasicInformationClusterReachableChangedEvent
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTROTASoftwareUpdateRequestorClusterProviderLocation : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull providerNodeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull endpoint API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterProviderLocation", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTROtaSoftwareUpdateRequestorClusterProviderLocation : MTROTASoftwareUpdateRequestorClusterProviderLocation
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTROTASoftwareUpdateRequestorClusterStateTransitionEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull previousState API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy, getter=getNewState)
    NSNumber * _Nonnull newState API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull reason API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy)
    NSNumber * _Nullable targetSoftwareVersion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterStateTransitionEvent", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTROtaSoftwareUpdateRequestorClusterStateTransitionEvent : MTROTASoftwareUpdateRequestorClusterStateTransitionEvent
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTROTASoftwareUpdateRequestorClusterVersionAppliedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull productID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterVersionAppliedEvent", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTROtaSoftwareUpdateRequestorClusterVersionAppliedEvent : MTROTASoftwareUpdateRequestorClusterVersionAppliedEvent
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTROTASoftwareUpdateRequestorClusterDownloadErrorEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull bytesDownloaded API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nullable progressPercent API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nullable platformCode API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterDownloadErrorEvent", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTROtaSoftwareUpdateRequestorClusterDownloadErrorEvent : MTROTASoftwareUpdateRequestorClusterDownloadErrorEvent
@end

MTR_DEPRECATED("This struct is unused and will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRPowerSourceClusterBatChargeFaultChangeType : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current MTR_DEPRECATED("Please use MTRPowerSourceClusterBatChargeFaultChangeType",
    ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSArray * _Nonnull previous MTR_DEPRECATED("Please use MTRPowerSourceClusterBatChargeFaultChangeType",
    ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_DEPRECATED("This struct is unused and will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRPowerSourceClusterBatFaultChangeType : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current MTR_DEPRECATED(
    "Please use MTRPowerSourceClusterBatFaultChangeType", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSArray * _Nonnull previous MTR_DEPRECATED(
    "Please use MTRPowerSourceClusterBatFaultChangeType", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_DEPRECATED("This struct is unused and will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRPowerSourceClusterWiredFaultChangeType : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current MTR_DEPRECATED("Please use MTRPowerSourceClusterWiredFaultChangeType",
    ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSArray * _Nonnull previous MTR_DEPRECATED("Please use MTRPowerSourceClusterWiredFaultChangeType",
    ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRPowerSourceClusterWiredFaultChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nonnull previous API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRPowerSourceClusterBatFaultChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nonnull previous API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRPowerSourceClusterBatChargeFaultChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nonnull previous API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
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

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRGeneralDiagnosticsClusterNetworkInterface : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull name API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull isOperational API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy)
    NSNumber * _Nullable offPremiseServicesReachableIPv4 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy)
    NSNumber * _Nullable offPremiseServicesReachableIPv6 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSData * _Nonnull hardwareAddress API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nonnull iPv4Addresses API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nonnull iPv6Addresses API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull type API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRGeneralDiagnosticsClusterNetworkInterface", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4),
    tvos(16.1, 16.4))
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

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRSoftwareDiagnosticsClusterThreadMetricsStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull id API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nullable name API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nullable stackFreeCurrent API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nullable stackFreeMinimum API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nullable stackSize API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRSoftwareDiagnosticsClusterThreadMetricsStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4),
    tvos(16.1, 16.4))
@interface MTRSoftwareDiagnosticsClusterThreadMetrics : MTRSoftwareDiagnosticsClusterThreadMetricsStruct
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRSoftwareDiagnosticsClusterSoftwareFaultEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull id API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSString * _Nullable name API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSData * _Nullable faultRecording API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_NEWLY_AVAILABLE
@interface MTRThreadNetworkDiagnosticsClusterNeighborTableStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull extAddress MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull age MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull rloc16 MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull linkFrameCounter MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull mleFrameCounter MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull lqi MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable averageRssi MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable lastRssi MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull frameErrorRate MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull messageErrorRate MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull rxOnWhenIdle MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull fullThreadDevice MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull fullNetworkData MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull isChild MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRThreadNetworkDiagnosticsClusterNeighborTableStruct")
@interface MTRThreadNetworkDiagnosticsClusterNeighborTable : MTRThreadNetworkDiagnosticsClusterNeighborTableStruct
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

MTR_NEWLY_AVAILABLE
@interface MTRThreadNetworkDiagnosticsClusterRouteTableStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull extAddress MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull rloc16 MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull routerId MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull nextHop MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull pathCost MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull lqiIn MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull lqiOut MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull age MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull allocated MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull linkEstablished MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRThreadNetworkDiagnosticsClusterRouteTableStruct")
@interface MTRThreadNetworkDiagnosticsClusterRouteTable : MTRThreadNetworkDiagnosticsClusterRouteTableStruct
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

API_AVAILABLE(ios(16.5), macos(13.4), watchos(9.5), tvos(16.5))
@interface MTRTimeSynchronizationClusterDSTOffsetStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull offset API_AVAILABLE(ios(16.5), macos(13.4), watchos(9.5), tvos(16.5));
@property (nonatomic, copy) NSNumber * _Nonnull validStarting API_AVAILABLE(ios(16.5), macos(13.4), watchos(9.5), tvos(16.5));
@property (nonatomic, copy) NSNumber * _Nullable validUntil API_AVAILABLE(ios(16.5), macos(13.4), watchos(9.5), tvos(16.5));
@end

MTR_DEPRECATED("Please use MTRTimeSynchronizationClusterDSTOffsetStruct", ios(16.1, 16.5), macos(13.0, 13.4), watchos(9.1, 9.5),
    tvos(16.1, 16.5))
@interface MTRTimeSynchronizationClusterDstOffsetType : MTRTimeSynchronizationClusterDSTOffsetStruct
@end
API_AVAILABLE(ios(16.5), macos(13.4), watchos(9.5), tvos(16.5))
@interface MTRTimeSynchronizationClusterTimeZoneStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull offset API_AVAILABLE(ios(16.5), macos(13.4), watchos(9.5), tvos(16.5));
@property (nonatomic, copy) NSNumber * _Nonnull validAt API_AVAILABLE(ios(16.5), macos(13.4), watchos(9.5), tvos(16.5));
@property (nonatomic, copy) NSString * _Nullable name API_AVAILABLE(ios(16.5), macos(13.4), watchos(9.5), tvos(16.5));
@end

MTR_DEPRECATED("Please use MTRTimeSynchronizationClusterTimeZoneStruct", ios(16.1, 16.5), macos(13.0, 13.4), watchos(9.1, 9.5),
    tvos(16.1, 16.5))
@interface MTRTimeSynchronizationClusterTimeZoneType : MTRTimeSynchronizationClusterTimeZoneStruct
@end

MTR_NEWLY_AVAILABLE
@interface MTRBridgedDeviceBasicInformationClusterProductAppearanceStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull finish MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable primaryColor MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRBridgedDeviceBasicInformationClusterStartUpEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRBridgedDeviceBasicInformationClusterStartUpEvent")
@interface MTRBridgedDeviceBasicClusterStartUpEvent : MTRBridgedDeviceBasicInformationClusterStartUpEvent
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRBridgedDeviceBasicInformationClusterShutDownEvent : NSObject <NSCopying>
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRBridgedDeviceBasicInformationClusterShutDownEvent")
@interface MTRBridgedDeviceBasicClusterShutDownEvent : MTRBridgedDeviceBasicInformationClusterShutDownEvent
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRBridgedDeviceBasicInformationClusterLeaveEvent : NSObject <NSCopying>
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRBridgedDeviceBasicInformationClusterLeaveEvent")
@interface MTRBridgedDeviceBasicClusterLeaveEvent : MTRBridgedDeviceBasicInformationClusterLeaveEvent
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRBridgedDeviceBasicInformationClusterReachableChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull reachableNewValue API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRBridgedDeviceBasicInformationClusterReachableChangedEvent")
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
@property (nonatomic, copy) NSNumber * _Nonnull previousPosition API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy, getter=getNewPosition) NSNumber * _Nonnull newPosition MTR_DEPRECATED(
    "Please use previousPosition", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy)
    NSNumber * _Nonnull totalNumberOfPressesCounted API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTROperationalCredentialsClusterFabricDescriptorStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSData * _Nonnull rootPublicKey API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull vendorID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull vendorId MTR_DEPRECATED(
    "Please use vendorID", ios(16.4, 16.4), macos(13.3, 13.3), watchos(9.4, 9.4), tvos(16.4, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull fabricID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull fabricId MTR_DEPRECATED(
    "Please use fabricID", ios(16.4, 16.4), macos(13.3, 13.3), watchos(9.4, 9.4), tvos(16.4, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull nodeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull nodeId MTR_DEPRECATED(
    "Please use nodeID", ios(16.4, 16.4), macos(13.3, 13.3), watchos(9.4, 9.4), tvos(16.4, 16.4));
@property (nonatomic, copy) NSString * _Nonnull label API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTROperationalCredentialsClusterFabricDescriptorStruct", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
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

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRModeSelectClusterSemanticTagStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull mfgCode API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull value API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED(
    "Please use MTRModeSelectClusterSemanticTagStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRModeSelectClusterSemanticTag : MTRModeSelectClusterSemanticTagStruct
@end
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRModeSelectClusterModeOptionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull label API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull mode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSArray * _Nonnull semanticTags API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_NEWLY_AVAILABLE
@interface MTRTemperatureControlClusterTemperatureLevelStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull label MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull temperatureLevel MTR_NEWLY_AVAILABLE;
@end

MTR_NEWLY_AVAILABLE
@interface MTRRefrigeratorAlarmClusterNotifyEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull active MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull inactive MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull state MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull mask MTR_NEWLY_AVAILABLE;
@end

MTR_NEWLY_AVAILABLE
@interface MTRSmokeCOAlarmClusterSmokeAlarmEvent : NSObject <NSCopying>
@end

MTR_NEWLY_AVAILABLE
@interface MTRSmokeCOAlarmClusterCOAlarmEvent : NSObject <NSCopying>
@end

MTR_NEWLY_AVAILABLE
@interface MTRSmokeCOAlarmClusterLowBatteryEvent : NSObject <NSCopying>
@end

MTR_NEWLY_AVAILABLE
@interface MTRSmokeCOAlarmClusterHardwareFaultEvent : NSObject <NSCopying>
@end

MTR_NEWLY_AVAILABLE
@interface MTRSmokeCOAlarmClusterEndOfServiceEvent : NSObject <NSCopying>
@end

MTR_NEWLY_AVAILABLE
@interface MTRSmokeCOAlarmClusterSelfTestCompleteEvent : NSObject <NSCopying>
@end

MTR_NEWLY_AVAILABLE
@interface MTRSmokeCOAlarmClusterAlarmMutedEvent : NSObject <NSCopying>
@end

MTR_NEWLY_AVAILABLE
@interface MTRSmokeCOAlarmClusterMuteEndedEvent : NSObject <NSCopying>
@end

MTR_NEWLY_AVAILABLE
@interface MTRSmokeCOAlarmClusterInterconnectSmokeAlarmEvent : NSObject <NSCopying>
@end

MTR_NEWLY_AVAILABLE
@interface MTRSmokeCOAlarmClusterInterconnectCOAlarmEvent : NSObject <NSCopying>
@end

MTR_NEWLY_AVAILABLE
@interface MTRSmokeCOAlarmClusterAllClearEvent : NSObject <NSCopying>
@end

MTR_NEWLY_AVAILABLE
@interface MTROperationalStateClusterErrorStateStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull errorStateID MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable errorStateLabel MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable errorStateDetails MTR_NEWLY_AVAILABLE;
@end

MTR_NEWLY_AVAILABLE
@interface MTROperationalStateClusterOperationalStateStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull operationalStateID MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nonnull operationalStateLabel MTR_NEWLY_AVAILABLE;
@end

MTR_NEWLY_AVAILABLE
@interface MTROperationalStateClusterOperationalErrorEvent : NSObject <NSCopying>
@property (nonatomic, copy) MTROperationalStateClusterErrorStateStruct * _Nonnull errorState MTR_NEWLY_AVAILABLE;
@end

MTR_NEWLY_AVAILABLE
@interface MTROperationalStateClusterOperationCompletionEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull completionErrorCode MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable totalOperationalTime MTR_NEWLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable pausedTime MTR_NEWLY_AVAILABLE;
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRDoorLockClusterCredentialStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull credentialType API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull credentialIndex API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED(
    "Please use MTRDoorLockClusterCredentialStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
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

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRChannelClusterChannelInfoStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull majorNumber API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull minorNumber API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nullable name API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nullable callSign API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nullable affiliateCallSign API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED(
    "Please use MTRChannelClusterChannelInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRChannelClusterChannelInfo : MTRChannelClusterChannelInfoStruct
@end
API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRChannelClusterLineupInfoStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull operatorName API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nullable lineupName API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nullable postalCode API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull lineupInfoType API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED(
    "Please use MTRChannelClusterLineupInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRChannelClusterLineupInfo : MTRChannelClusterLineupInfoStruct
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRTargetNavigatorClusterTargetInfoStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull identifier API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nonnull name API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED(
    "Please use MTRTargetNavigatorClusterTargetInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTargetNavigatorClusterTargetInfo : MTRTargetNavigatorClusterTargetInfoStruct
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRMediaPlaybackClusterPlaybackPositionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull updatedAt API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nullable position API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRMediaPlaybackClusterPlaybackPositionStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4),
    tvos(16.1, 16.4))
@interface MTRMediaPlaybackClusterPlaybackPosition : MTRMediaPlaybackClusterPlaybackPositionStruct
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRMediaInputClusterInputInfoStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull index API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull inputType API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nonnull name API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nonnull descriptionString API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED(
    "Please use MTRMediaInputClusterInputInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRMediaInputClusterInputInfo : MTRMediaInputClusterInputInfoStruct
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRContentLauncherClusterDimensionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull width API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull height API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull metric API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED(
    "Please use MTRContentLauncherClusterDimensionStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRContentLauncherClusterDimension : MTRContentLauncherClusterDimensionStruct
@end
API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRContentLauncherClusterAdditionalInfoStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull name API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nonnull value API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRContentLauncherClusterAdditionalInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4),
    tvos(16.1, 16.4))
@interface MTRContentLauncherClusterAdditionalInfo : MTRContentLauncherClusterAdditionalInfoStruct
@end
API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRContentLauncherClusterParameterStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull type API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nonnull value API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nullable externalIDList API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED(
    "Please use MTRContentLauncherClusterParameterStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRContentLauncherClusterParameter : MTRContentLauncherClusterParameterStruct
@end
API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRContentLauncherClusterContentSearchStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull parameterList API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRContentLauncherClusterContentSearchStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4),
    tvos(16.1, 16.4))
@interface MTRContentLauncherClusterContentSearch : MTRContentLauncherClusterContentSearchStruct
@end
API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRContentLauncherClusterStyleInformationStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nullable imageURL API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nullable imageUrl MTR_DEPRECATED(
    "Please use imageURL", ios(16.4, 16.4), macos(13.3, 13.3), watchos(9.4, 9.4), tvos(16.4, 16.4));
@property (nonatomic, copy) NSString * _Nullable color API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy)
    MTRContentLauncherClusterDimensionStruct * _Nullable size API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRContentLauncherClusterStyleInformationStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4),
    tvos(16.1, 16.4))
@interface MTRContentLauncherClusterStyleInformation : MTRContentLauncherClusterStyleInformationStruct
@end
API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRContentLauncherClusterBrandingInformationStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull providerName API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) MTRContentLauncherClusterStyleInformationStruct * _Nullable background API_AVAILABLE(
    ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) MTRContentLauncherClusterStyleInformationStruct * _Nullable logo API_AVAILABLE(
    ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) MTRContentLauncherClusterStyleInformationStruct * _Nullable progressBar API_AVAILABLE(
    ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) MTRContentLauncherClusterStyleInformationStruct * _Nullable splash API_AVAILABLE(
    ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) MTRContentLauncherClusterStyleInformationStruct * _Nullable waterMark API_AVAILABLE(
    ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRContentLauncherClusterBrandingInformationStruct", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRContentLauncherClusterBrandingInformation : MTRContentLauncherClusterBrandingInformationStruct
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRAudioOutputClusterOutputInfoStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull index API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull outputType API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nonnull name API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED(
    "Please use MTRAudioOutputClusterOutputInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRAudioOutputClusterOutputInfo : MTRAudioOutputClusterOutputInfoStruct
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRApplicationLauncherClusterApplicationStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull catalogVendorID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull catalogVendorId MTR_DEPRECATED(
    "Please use catalogVendorID", ios(16.4, 16.4), macos(13.3, 13.3), watchos(9.4, 9.4), tvos(16.4, 16.4));
@property (nonatomic, copy) NSString * _Nonnull applicationID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nonnull applicationId MTR_DEPRECATED(
    "Please use applicationID", ios(16.4, 16.4), macos(13.3, 13.3), watchos(9.4, 9.4), tvos(16.4, 16.4));
@end

MTR_DEPRECATED("Please use MTRApplicationLauncherClusterApplicationStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4),
    tvos(16.1, 16.4))
@interface MTRApplicationLauncherClusterApplication : MTRApplicationLauncherClusterApplicationStruct
@end
API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRApplicationLauncherClusterApplicationEPStruct : NSObject <NSCopying>
@property (nonatomic, copy) MTRApplicationLauncherClusterApplicationStruct * _Nonnull application API_AVAILABLE(
    ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nullable endpoint API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRApplicationLauncherClusterApplicationEPStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4),
    tvos(16.1, 16.4))
@interface MTRApplicationLauncherClusterApplicationEP : MTRApplicationLauncherClusterApplicationEPStruct
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRApplicationBasicClusterApplicationStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull catalogVendorID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull catalogVendorId MTR_DEPRECATED(
    "Please use catalogVendorID", ios(16.4, 16.4), macos(13.3, 13.3), watchos(9.4, 9.4), tvos(16.4, 16.4));
@property (nonatomic, copy) NSString * _Nonnull applicationID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nonnull applicationId MTR_DEPRECATED(
    "Please use applicationID", ios(16.4, 16.4), macos(13.3, 13.3), watchos(9.4, 9.4), tvos(16.4, 16.4));
@end

MTR_DEPRECATED("Please use MTRApplicationBasicClusterApplicationStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4),
    tvos(16.1, 16.4))
@interface MTRApplicationBasicClusterApplicationBasicApplication : MTRApplicationBasicClusterApplicationStruct
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterSimpleStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull a API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull b API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull c API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSData * _Nonnull d API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nonnull e API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull f API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull g API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull h API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED(
    "Please use MTRUnitTestingClusterSimpleStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterSimpleStruct : MTRUnitTestingClusterSimpleStruct
@end
API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestFabricScoped : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull fabricSensitiveInt8u API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    ;
@property (nonatomic, copy)
    NSNumber * _Nullable optionalFabricSensitiveInt8u API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy)
    NSNumber * _Nullable nullableFabricSensitiveInt8u API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy)
    NSNumber * _Nullable nullableOptionalFabricSensitiveInt8u API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy)
    NSString * _Nonnull fabricSensitiveCharString API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nonnull fabricSensitiveStruct API_AVAILABLE(
    ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy)
    NSArray * _Nonnull fabricSensitiveInt8uList API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED(
    "Please use MTRUnitTestingClusterTestFabricScoped", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestFabricScoped : MTRUnitTestingClusterTestFabricScoped
@end
API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterNullablesAndOptionalsStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable nullableInt API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nullable optionalInt API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalInt API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    ;
@property (nonatomic, copy) NSString * _Nullable nullableString API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nullable optionalString API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy)
    NSString * _Nullable nullableOptionalString API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy)
    MTRUnitTestingClusterSimpleStruct * _Nullable nullableStruct API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy)
    MTRUnitTestingClusterSimpleStruct * _Nullable optionalStruct API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable nullableOptionalStruct API_AVAILABLE(
    ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nullable nullableList API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nullable optionalList API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nullable nullableOptionalList API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    ;
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterNullablesAndOptionalsStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4),
    tvos(16.1, 16.4))
@interface MTRTestClusterClusterNullablesAndOptionalsStruct : MTRUnitTestingClusterNullablesAndOptionalsStruct
@end
API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterNestedStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull a API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull b API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy)
    MTRUnitTestingClusterSimpleStruct * _Nonnull c API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED(
    "Please use MTRUnitTestingClusterNestedStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterNestedStruct : MTRUnitTestingClusterNestedStruct
@end
API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterNestedStructList : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull a API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull b API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy)
    MTRUnitTestingClusterSimpleStruct * _Nonnull c API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nonnull d API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nonnull e API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nonnull f API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nonnull g API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED(
    "Please use MTRUnitTestingClusterNestedStructList", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterNestedStructList : MTRUnitTestingClusterNestedStructList
@end
API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterDoubleNestedStructList : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull a API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterDoubleNestedStructList", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4),
    tvos(16.1, 16.4))
@interface MTRTestClusterClusterDoubleNestedStructList : MTRUnitTestingClusterDoubleNestedStructList
@end
API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestListStructOctet : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull member1 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSData * _Nonnull member2 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED(
    "Please use MTRUnitTestingClusterTestListStructOctet", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestListStructOctet : MTRUnitTestingClusterTestListStructOctet
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestEventEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull arg1 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull arg2 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull arg3 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy)
    MTRUnitTestingClusterSimpleStruct * _Nonnull arg4 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nonnull arg5 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nonnull arg6 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED(
    "Please use MTRUnitTestingClusterTestEventEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestEventEvent : MTRUnitTestingClusterTestEventEvent
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestFabricScopedEventEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestFabricScopedEventEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4),
    tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestFabricScopedEventEvent : MTRUnitTestingClusterTestFabricScopedEventEvent
@end

NS_ASSUME_NONNULL_END
