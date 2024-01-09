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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterAttributeValuePair : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull attributeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull attributeId MTR_DEPRECATED("Please use attributeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull attributeValue MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterExtensionFieldSet : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull clusterID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull clusterId MTR_DEPRECATED("Please use clusterID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSArray * _Nonnull attributeValueList MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRScenesClusterSceneInfoStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull sceneCount MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull currentScene MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull currentGroup MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull sceneValid MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull remainingCapacity MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_AVAILABLE(ios(16.2), macos(13.1), watchos(9.2), tvos(16.2))
@interface MTRDescriptorClusterDeviceTypeStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull deviceType MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull type MTR_DEPRECATED("Please use deviceType", ios(16.2, 16.4), macos(13.1, 13.3), watchos(9.2, 9.4), tvos(16.2, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull revision MTR_AVAILABLE(ios(16.2), macos(13.1), watchos(9.2), tvos(16.2));
@end

MTR_DEPRECATED("Please use MTRDescriptorClusterDeviceTypeStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRDescriptorClusterDeviceType : MTRDescriptorClusterDeviceTypeStruct
@property (nonatomic, copy) NSNumber * _Nonnull revision MTR_DEPRECATED("Please use MTRDescriptorClusterDeviceTypeStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRDescriptorClusterSemanticTagStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable mfgCode MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull namespaceID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull tag MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable label MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRBindingClusterTargetStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable node MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable group MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable endpoint MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable cluster MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0))
@interface MTRAccessControlClusterAccessControlTargetStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable cluster MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nullable endpoint MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nullable deviceType MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_DEPRECATED("Please use MTRAccessControlClusterAccessControlTargetStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0))
@interface MTRAccessControlClusterTarget : MTRAccessControlClusterAccessControlTargetStruct
@property (nonatomic, copy) NSNumber * _Nullable cluster MTR_DEPRECATED("Please use MTRAccessControlClusterAccessControlTargetStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nullable endpoint MTR_DEPRECATED("Please use MTRAccessControlClusterAccessControlTargetStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nullable deviceType MTR_DEPRECATED("Please use MTRAccessControlClusterAccessControlTargetStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRAccessControlClusterAccessControlEntryStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull privilege MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull authMode MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nullable subjects MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nullable targets MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRAccessControlClusterAccessControlEntryStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRAccessControlClusterAccessControlEntry : MTRAccessControlClusterAccessControlEntryStruct
@property (nonatomic, copy) NSNumber * _Nonnull privilege MTR_DEPRECATED("Please use MTRAccessControlClusterAccessControlEntryStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull authMode MTR_DEPRECATED("Please use MTRAccessControlClusterAccessControlEntryStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSArray * _Nullable subjects MTR_DEPRECATED("Please use MTRAccessControlClusterAccessControlEntryStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSArray * _Nullable targets MTR_DEPRECATED("Please use MTRAccessControlClusterAccessControlEntryStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_DEPRECATED("Please use MTRAccessControlClusterAccessControlEntryStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRAccessControlClusterAccessControlExtensionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSData * _Nonnull data MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRAccessControlClusterAccessControlExtensionStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRAccessControlClusterExtensionEntry : MTRAccessControlClusterAccessControlExtensionStruct
@property (nonatomic, copy) NSData * _Nonnull data MTR_DEPRECATED("Please use MTRAccessControlClusterAccessControlExtensionStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_DEPRECATED("Please use MTRAccessControlClusterAccessControlExtensionStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRAccessControlClusterAccessControlEntryChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable adminNodeID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable adminPasscodeID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull changeType MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) MTRAccessControlClusterAccessControlEntryStruct * _Nullable latestValue MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRAccessControlClusterAccessControlExtensionChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable adminNodeID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable adminPasscodeID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull changeType MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) MTRAccessControlClusterAccessControlExtensionStruct * _Nullable latestValue MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterActionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull actionID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSString * _Nonnull name MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull type MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull endpointListID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull supportedCommands MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull state MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterEndpointListStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull endpointListID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSString * _Nonnull name MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull type MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSArray * _Nonnull endpoints MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterStateChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull actionID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull invokeID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy, getter=getNewState) NSNumber * _Nonnull newState MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterActionFailedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull actionID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull invokeID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy, getter=getNewState) NSNumber * _Nonnull newState MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull error MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRBasicInformationClusterCapabilityMinimaStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull caseSessionsPerFabric MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull subscriptionsPerFabric MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRBasicInformationClusterCapabilityMinimaStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRBasicClusterCapabilityMinimaStruct : MTRBasicInformationClusterCapabilityMinimaStruct
@property (nonatomic, copy) NSNumber * _Nonnull caseSessionsPerFabric MTR_DEPRECATED("Please use MTRBasicInformationClusterCapabilityMinimaStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull subscriptionsPerFabric MTR_DEPRECATED("Please use MTRBasicInformationClusterCapabilityMinimaStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0))
@interface MTRBasicInformationClusterProductAppearanceStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull finish MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nullable primaryColor MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRBasicInformationClusterStartUpEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRBasicInformationClusterStartUpEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRBasicClusterStartUpEvent : MTRBasicInformationClusterStartUpEvent
@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion MTR_DEPRECATED("Please use MTRBasicInformationClusterStartUpEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRBasicInformationClusterShutDownEvent : NSObject <NSCopying>
@end

MTR_DEPRECATED("Please use MTRBasicInformationClusterShutDownEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRBasicClusterShutDownEvent : MTRBasicInformationClusterShutDownEvent
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRBasicInformationClusterLeaveEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRBasicInformationClusterLeaveEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRBasicClusterLeaveEvent : MTRBasicInformationClusterLeaveEvent
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_DEPRECATED("Please use MTRBasicInformationClusterLeaveEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRBasicInformationClusterReachableChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull reachableNewValue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRBasicInformationClusterReachableChangedEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRBasicClusterReachableChangedEvent : MTRBasicInformationClusterReachableChangedEvent
@property (nonatomic, copy) NSNumber * _Nonnull reachableNewValue MTR_DEPRECATED("Please use MTRBasicInformationClusterReachableChangedEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTROTASoftwareUpdateRequestorClusterProviderLocation : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull providerNodeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull endpoint MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterProviderLocation", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTROtaSoftwareUpdateRequestorClusterProviderLocation : MTROTASoftwareUpdateRequestorClusterProviderLocation
@property (nonatomic, copy) NSNumber * _Nonnull providerNodeID MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterProviderLocation", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull endpoint MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterProviderLocation", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterProviderLocation", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTROTASoftwareUpdateRequestorClusterStateTransitionEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull previousState MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy, getter=getNewState) NSNumber * _Nonnull newState MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull reason MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nullable targetSoftwareVersion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterStateTransitionEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTROtaSoftwareUpdateRequestorClusterStateTransitionEvent : MTROTASoftwareUpdateRequestorClusterStateTransitionEvent
@property (nonatomic, copy) NSNumber * _Nonnull previousState MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterStateTransitionEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy, getter=getNewState) NSNumber * _Nonnull newState MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterStateTransitionEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull reason MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterStateTransitionEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nullable targetSoftwareVersion MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterStateTransitionEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTROTASoftwareUpdateRequestorClusterVersionAppliedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull productID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterVersionAppliedEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTROtaSoftwareUpdateRequestorClusterVersionAppliedEvent : MTROTASoftwareUpdateRequestorClusterVersionAppliedEvent
@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterVersionAppliedEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull productID MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterVersionAppliedEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTROTASoftwareUpdateRequestorClusterDownloadErrorEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull bytesDownloaded MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nullable progressPercent MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nullable platformCode MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterDownloadErrorEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTROtaSoftwareUpdateRequestorClusterDownloadErrorEvent : MTROTASoftwareUpdateRequestorClusterDownloadErrorEvent
@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterDownloadErrorEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull bytesDownloaded MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterDownloadErrorEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nullable progressPercent MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterDownloadErrorEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nullable platformCode MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterDownloadErrorEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_DEPRECATED("This struct is unused and will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRPowerSourceClusterBatChargeFaultChangeType : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current MTR_DEPRECATED("Please use MTRPowerSourceClusterBatChargeFaultChangeType", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSArray * _Nonnull previous MTR_DEPRECATED("Please use MTRPowerSourceClusterBatChargeFaultChangeType", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_DEPRECATED("This struct is unused and will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRPowerSourceClusterBatFaultChangeType : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current MTR_DEPRECATED("Please use MTRPowerSourceClusterBatFaultChangeType", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSArray * _Nonnull previous MTR_DEPRECATED("Please use MTRPowerSourceClusterBatFaultChangeType", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_DEPRECATED("This struct is unused and will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRPowerSourceClusterWiredFaultChangeType : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current MTR_DEPRECATED("Please use MTRPowerSourceClusterWiredFaultChangeType", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSArray * _Nonnull previous MTR_DEPRECATED("Please use MTRPowerSourceClusterWiredFaultChangeType", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRPowerSourceClusterWiredFaultChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nonnull previous MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRPowerSourceClusterBatFaultChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nonnull previous MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRPowerSourceClusterBatChargeFaultChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nonnull previous MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGeneralCommissioningClusterBasicCommissioningInfo : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull failSafeExpiryLengthSeconds MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull maxCumulativeFailsafeSeconds MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0))
@interface MTRNetworkCommissioningClusterNetworkInfoStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSData * _Nonnull networkID MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nonnull connected MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSData * _Nullable networkIdentifier MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSData * _Nullable clientIdentifier MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_DEPRECATED("Please use MTRNetworkCommissioningClusterNetworkInfoStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0))
@interface MTRNetworkCommissioningClusterNetworkInfo : MTRNetworkCommissioningClusterNetworkInfoStruct
@property (nonatomic, copy) NSData * _Nonnull networkID MTR_DEPRECATED("Please use MTRNetworkCommissioningClusterNetworkInfoStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nonnull connected MTR_DEPRECATED("Please use MTRNetworkCommissioningClusterNetworkInfoStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@end

MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0))
@interface MTRNetworkCommissioningClusterThreadInterfaceScanResultStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull panId MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nonnull extendedPanId MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSString * _Nonnull networkName MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nonnull channel MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nonnull version MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSData * _Nonnull extendedAddress MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nonnull rssi MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nonnull lqi MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_DEPRECATED("Please use MTRNetworkCommissioningClusterThreadInterfaceScanResultStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0))
@interface MTRNetworkCommissioningClusterThreadInterfaceScanResult : MTRNetworkCommissioningClusterThreadInterfaceScanResultStruct
@property (nonatomic, copy) NSNumber * _Nonnull panId MTR_DEPRECATED("Please use MTRNetworkCommissioningClusterThreadInterfaceScanResultStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nonnull extendedPanId MTR_DEPRECATED("Please use MTRNetworkCommissioningClusterThreadInterfaceScanResultStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSString * _Nonnull networkName MTR_DEPRECATED("Please use MTRNetworkCommissioningClusterThreadInterfaceScanResultStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nonnull channel MTR_DEPRECATED("Please use MTRNetworkCommissioningClusterThreadInterfaceScanResultStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nonnull version MTR_DEPRECATED("Please use MTRNetworkCommissioningClusterThreadInterfaceScanResultStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSData * _Nonnull extendedAddress MTR_DEPRECATED("Please use MTRNetworkCommissioningClusterThreadInterfaceScanResultStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nonnull rssi MTR_DEPRECATED("Please use MTRNetworkCommissioningClusterThreadInterfaceScanResultStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nonnull lqi MTR_DEPRECATED("Please use MTRNetworkCommissioningClusterThreadInterfaceScanResultStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@end

MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0))
@interface MTRNetworkCommissioningClusterWiFiInterfaceScanResultStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull security MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSData * _Nonnull ssid MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSData * _Nonnull bssid MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nonnull channel MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nonnull wiFiBand MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nonnull rssi MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_DEPRECATED("Please use MTRNetworkCommissioningClusterWiFiInterfaceScanResultStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0))
@interface MTRNetworkCommissioningClusterWiFiInterfaceScanResult : MTRNetworkCommissioningClusterWiFiInterfaceScanResultStruct
@property (nonatomic, copy) NSNumber * _Nonnull security MTR_DEPRECATED("Please use MTRNetworkCommissioningClusterWiFiInterfaceScanResultStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSData * _Nonnull ssid MTR_DEPRECATED("Please use MTRNetworkCommissioningClusterWiFiInterfaceScanResultStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSData * _Nonnull bssid MTR_DEPRECATED("Please use MTRNetworkCommissioningClusterWiFiInterfaceScanResultStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nonnull channel MTR_DEPRECATED("Please use MTRNetworkCommissioningClusterWiFiInterfaceScanResultStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nonnull wiFiBand MTR_DEPRECATED("Please use MTRNetworkCommissioningClusterWiFiInterfaceScanResultStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nonnull rssi MTR_DEPRECATED("Please use MTRNetworkCommissioningClusterWiFiInterfaceScanResultStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRGeneralDiagnosticsClusterNetworkInterface : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull name MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull isOperational MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nullable offPremiseServicesReachableIPv4 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nullable offPremiseServicesReachableIPv6 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSData * _Nonnull hardwareAddress MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nonnull iPv4Addresses MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nonnull iPv6Addresses MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull type MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRGeneralDiagnosticsClusterNetworkInterface", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRGeneralDiagnosticsClusterNetworkInterfaceType : MTRGeneralDiagnosticsClusterNetworkInterface
@property (nonatomic, copy) NSString * _Nonnull name MTR_DEPRECATED("Please use MTRGeneralDiagnosticsClusterNetworkInterface", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull isOperational MTR_DEPRECATED("Please use MTRGeneralDiagnosticsClusterNetworkInterface", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nullable offPremiseServicesReachableIPv4 MTR_DEPRECATED("Please use MTRGeneralDiagnosticsClusterNetworkInterface", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nullable offPremiseServicesReachableIPv6 MTR_DEPRECATED("Please use MTRGeneralDiagnosticsClusterNetworkInterface", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSData * _Nonnull hardwareAddress MTR_DEPRECATED("Please use MTRGeneralDiagnosticsClusterNetworkInterface", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSArray * _Nonnull iPv4Addresses MTR_DEPRECATED("Please use MTRGeneralDiagnosticsClusterNetworkInterface", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSArray * _Nonnull iPv6Addresses MTR_DEPRECATED("Please use MTRGeneralDiagnosticsClusterNetworkInterface", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull type MTR_DEPRECATED("Please use MTRGeneralDiagnosticsClusterNetworkInterface", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGeneralDiagnosticsClusterHardwareFaultChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSArray * _Nonnull previous MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGeneralDiagnosticsClusterRadioFaultChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSArray * _Nonnull previous MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGeneralDiagnosticsClusterNetworkFaultChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSArray * _Nonnull previous MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGeneralDiagnosticsClusterBootReasonEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull bootReason MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRSoftwareDiagnosticsClusterThreadMetricsStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull id MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nullable name MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nullable stackFreeCurrent MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nullable stackFreeMinimum MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nullable stackSize MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRSoftwareDiagnosticsClusterThreadMetricsStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRSoftwareDiagnosticsClusterThreadMetrics : MTRSoftwareDiagnosticsClusterThreadMetricsStruct
@property (nonatomic, copy) NSNumber * _Nonnull id MTR_DEPRECATED("Please use MTRSoftwareDiagnosticsClusterThreadMetricsStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSString * _Nullable name MTR_DEPRECATED("Please use MTRSoftwareDiagnosticsClusterThreadMetricsStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nullable stackFreeCurrent MTR_DEPRECATED("Please use MTRSoftwareDiagnosticsClusterThreadMetricsStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nullable stackFreeMinimum MTR_DEPRECATED("Please use MTRSoftwareDiagnosticsClusterThreadMetricsStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nullable stackSize MTR_DEPRECATED("Please use MTRSoftwareDiagnosticsClusterThreadMetricsStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRSoftwareDiagnosticsClusterSoftwareFaultEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull id MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSString * _Nullable name MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSData * _Nullable faultRecording MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0))
@interface MTRThreadNetworkDiagnosticsClusterNeighborTableStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull extAddress MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nonnull age MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nonnull rloc16 MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nonnull linkFrameCounter MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nonnull mleFrameCounter MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nonnull lqi MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nullable averageRssi MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nullable lastRssi MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nonnull frameErrorRate MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nonnull messageErrorRate MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nonnull rxOnWhenIdle MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nonnull fullThreadDevice MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nonnull fullNetworkData MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nonnull isChild MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_DEPRECATED("Please use MTRThreadNetworkDiagnosticsClusterNeighborTableStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0))
@interface MTRThreadNetworkDiagnosticsClusterNeighborTable : MTRThreadNetworkDiagnosticsClusterNeighborTableStruct
@property (nonatomic, copy) NSNumber * _Nonnull extAddress MTR_DEPRECATED("Please use MTRThreadNetworkDiagnosticsClusterNeighborTableStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nonnull age MTR_DEPRECATED("Please use MTRThreadNetworkDiagnosticsClusterNeighborTableStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nonnull rloc16 MTR_DEPRECATED("Please use MTRThreadNetworkDiagnosticsClusterNeighborTableStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nonnull linkFrameCounter MTR_DEPRECATED("Please use MTRThreadNetworkDiagnosticsClusterNeighborTableStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nonnull mleFrameCounter MTR_DEPRECATED("Please use MTRThreadNetworkDiagnosticsClusterNeighborTableStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nonnull lqi MTR_DEPRECATED("Please use MTRThreadNetworkDiagnosticsClusterNeighborTableStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nullable averageRssi MTR_DEPRECATED("Please use MTRThreadNetworkDiagnosticsClusterNeighborTableStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nullable lastRssi MTR_DEPRECATED("Please use MTRThreadNetworkDiagnosticsClusterNeighborTableStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nonnull frameErrorRate MTR_DEPRECATED("Please use MTRThreadNetworkDiagnosticsClusterNeighborTableStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nonnull messageErrorRate MTR_DEPRECATED("Please use MTRThreadNetworkDiagnosticsClusterNeighborTableStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nonnull rxOnWhenIdle MTR_DEPRECATED("Please use MTRThreadNetworkDiagnosticsClusterNeighborTableStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nonnull fullThreadDevice MTR_DEPRECATED("Please use MTRThreadNetworkDiagnosticsClusterNeighborTableStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nonnull fullNetworkData MTR_DEPRECATED("Please use MTRThreadNetworkDiagnosticsClusterNeighborTableStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nonnull isChild MTR_DEPRECATED("Please use MTRThreadNetworkDiagnosticsClusterNeighborTableStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRThreadNetworkDiagnosticsClusterOperationalDatasetComponents : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull activeTimestampPresent MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull pendingTimestampPresent MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull masterKeyPresent MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull networkNamePresent MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull extendedPanIdPresent MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull meshLocalPrefixPresent MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull delayPresent MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull panIdPresent MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull channelPresent MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull pskcPresent MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull securityPolicyPresent MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull channelMaskPresent MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0))
@interface MTRThreadNetworkDiagnosticsClusterRouteTableStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull extAddress MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nonnull rloc16 MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nonnull routerId MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nonnull nextHop MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nonnull pathCost MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nonnull lqiIn MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nonnull lqiOut MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nonnull age MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nonnull allocated MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nonnull linkEstablished MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_DEPRECATED("Please use MTRThreadNetworkDiagnosticsClusterRouteTableStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0))
@interface MTRThreadNetworkDiagnosticsClusterRouteTable : MTRThreadNetworkDiagnosticsClusterRouteTableStruct
@property (nonatomic, copy) NSNumber * _Nonnull extAddress MTR_DEPRECATED("Please use MTRThreadNetworkDiagnosticsClusterRouteTableStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nonnull rloc16 MTR_DEPRECATED("Please use MTRThreadNetworkDiagnosticsClusterRouteTableStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nonnull routerId MTR_DEPRECATED("Please use MTRThreadNetworkDiagnosticsClusterRouteTableStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nonnull nextHop MTR_DEPRECATED("Please use MTRThreadNetworkDiagnosticsClusterRouteTableStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nonnull pathCost MTR_DEPRECATED("Please use MTRThreadNetworkDiagnosticsClusterRouteTableStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nonnull lqiIn MTR_DEPRECATED("Please use MTRThreadNetworkDiagnosticsClusterRouteTableStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nonnull lqiOut MTR_DEPRECATED("Please use MTRThreadNetworkDiagnosticsClusterRouteTableStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nonnull age MTR_DEPRECATED("Please use MTRThreadNetworkDiagnosticsClusterRouteTableStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nonnull allocated MTR_DEPRECATED("Please use MTRThreadNetworkDiagnosticsClusterRouteTableStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@property (nonatomic, copy) NSNumber * _Nonnull linkEstablished MTR_DEPRECATED("Please use MTRThreadNetworkDiagnosticsClusterRouteTableStruct", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRThreadNetworkDiagnosticsClusterSecurityPolicy : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull rotationTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull flags MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRThreadNetworkDiagnosticsClusterConnectionStatusEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull connectionStatus MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRThreadNetworkDiagnosticsClusterNetworkFaultChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSArray * _Nonnull previous MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRWiFiNetworkDiagnosticsClusterDisconnectionEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull reasonCode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRWiFiNetworkDiagnosticsClusterAssociationFailureEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull associationFailureCause MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@property (nonatomic, copy) NSNumber * _Nonnull associationFailure MTR_DEPRECATED("Please use associationFailureCause", ios(16.1, 17.4), macos(13.0, 14.4), watchos(9.1, 10.4), tvos(16.1, 17.4));
@property (nonatomic, copy) NSNumber * _Nonnull status MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRWiFiNetworkDiagnosticsClusterConnectionStatusEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull connectionStatus MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.5), macos(13.4), watchos(9.5), tvos(16.5))
@interface MTRTimeSynchronizationClusterDSTOffsetStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull offset MTR_AVAILABLE(ios(16.5), macos(13.4), watchos(9.5), tvos(16.5));
@property (nonatomic, copy) NSNumber * _Nonnull validStarting MTR_AVAILABLE(ios(16.5), macos(13.4), watchos(9.5), tvos(16.5));
@property (nonatomic, copy) NSNumber * _Nullable validUntil MTR_AVAILABLE(ios(16.5), macos(13.4), watchos(9.5), tvos(16.5));
@end

MTR_DEPRECATED("Please use MTRTimeSynchronizationClusterDSTOffsetStruct", ios(16.1, 16.5), macos(13.0, 13.4), watchos(9.1, 9.5), tvos(16.1, 16.5))
@interface MTRTimeSynchronizationClusterDstOffsetType : MTRTimeSynchronizationClusterDSTOffsetStruct
@property (nonatomic, copy) NSNumber * _Nonnull offset MTR_DEPRECATED("Please use MTRTimeSynchronizationClusterDSTOffsetStruct", ios(16.1, 16.5), macos(13.0, 13.4), watchos(9.1, 9.5), tvos(16.1, 16.5));
@property (nonatomic, copy) NSNumber * _Nonnull validStarting MTR_DEPRECATED("Please use MTRTimeSynchronizationClusterDSTOffsetStruct", ios(16.1, 16.5), macos(13.0, 13.4), watchos(9.1, 9.5), tvos(16.1, 16.5));
@property (nonatomic, copy) NSNumber * _Nullable validUntil MTR_DEPRECATED("Please use MTRTimeSynchronizationClusterDSTOffsetStruct", ios(16.1, 16.5), macos(13.0, 13.4), watchos(9.1, 9.5), tvos(16.1, 16.5));
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRTimeSynchronizationClusterFabricScopedTrustedTimeSourceStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull nodeID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull endpoint MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_AVAILABLE(ios(16.5), macos(13.4), watchos(9.5), tvos(16.5))
@interface MTRTimeSynchronizationClusterTimeZoneStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull offset MTR_AVAILABLE(ios(16.5), macos(13.4), watchos(9.5), tvos(16.5));
@property (nonatomic, copy) NSNumber * _Nonnull validAt MTR_AVAILABLE(ios(16.5), macos(13.4), watchos(9.5), tvos(16.5));
@property (nonatomic, copy) NSString * _Nullable name MTR_AVAILABLE(ios(16.5), macos(13.4), watchos(9.5), tvos(16.5));
@end

MTR_DEPRECATED("Please use MTRTimeSynchronizationClusterTimeZoneStruct", ios(16.1, 16.5), macos(13.0, 13.4), watchos(9.1, 9.5), tvos(16.1, 16.5))
@interface MTRTimeSynchronizationClusterTimeZoneType : MTRTimeSynchronizationClusterTimeZoneStruct
@property (nonatomic, copy) NSNumber * _Nonnull offset MTR_DEPRECATED("Please use MTRTimeSynchronizationClusterTimeZoneStruct", ios(16.1, 16.5), macos(13.0, 13.4), watchos(9.1, 9.5), tvos(16.1, 16.5));
@property (nonatomic, copy) NSNumber * _Nonnull validAt MTR_DEPRECATED("Please use MTRTimeSynchronizationClusterTimeZoneStruct", ios(16.1, 16.5), macos(13.0, 13.4), watchos(9.1, 9.5), tvos(16.1, 16.5));
@property (nonatomic, copy) NSString * _Nullable name MTR_DEPRECATED("Please use MTRTimeSynchronizationClusterTimeZoneStruct", ios(16.1, 16.5), macos(13.0, 13.4), watchos(9.1, 9.5), tvos(16.1, 16.5));
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRTimeSynchronizationClusterTrustedTimeSourceStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull nodeID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull endpoint MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRTimeSynchronizationClusterDSTTableEmptyEvent : NSObject <NSCopying>
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRTimeSynchronizationClusterDSTStatusEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull dstOffsetActive MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRTimeSynchronizationClusterTimeZoneStatusEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull offset MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable name MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRTimeSynchronizationClusterTimeFailureEvent : NSObject <NSCopying>
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRTimeSynchronizationClusterMissingTrustedTimeSourceEvent : NSObject <NSCopying>
@end

MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0))
@interface MTRBridgedDeviceBasicInformationClusterProductAppearanceStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull finish MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@property (nonatomic, copy) NSNumber * _Nullable primaryColor MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRBridgedDeviceBasicInformationClusterStartUpEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRBridgedDeviceBasicInformationClusterStartUpEvent", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0))
@interface MTRBridgedDeviceBasicClusterStartUpEvent : MTRBridgedDeviceBasicInformationClusterStartUpEvent
@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion MTR_DEPRECATED("Please use MTRBridgedDeviceBasicInformationClusterStartUpEvent", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRBridgedDeviceBasicInformationClusterShutDownEvent : NSObject <NSCopying>
@end

MTR_DEPRECATED("Please use MTRBridgedDeviceBasicInformationClusterShutDownEvent", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0))
@interface MTRBridgedDeviceBasicClusterShutDownEvent : MTRBridgedDeviceBasicInformationClusterShutDownEvent
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRBridgedDeviceBasicInformationClusterLeaveEvent : NSObject <NSCopying>
@end

MTR_DEPRECATED("Please use MTRBridgedDeviceBasicInformationClusterLeaveEvent", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0))
@interface MTRBridgedDeviceBasicClusterLeaveEvent : MTRBridgedDeviceBasicInformationClusterLeaveEvent
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRBridgedDeviceBasicInformationClusterReachableChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull reachableNewValue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRBridgedDeviceBasicInformationClusterReachableChangedEvent", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0))
@interface MTRBridgedDeviceBasicClusterReachableChangedEvent : MTRBridgedDeviceBasicInformationClusterReachableChangedEvent
@property (nonatomic, copy) NSNumber * _Nonnull reachableNewValue MTR_DEPRECATED("Please use MTRBridgedDeviceBasicInformationClusterReachableChangedEvent", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRSwitchClusterSwitchLatchedEvent : NSObject <NSCopying>
@property (nonatomic, copy, getter=getNewPosition) NSNumber * _Nonnull newPosition MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRSwitchClusterInitialPressEvent : NSObject <NSCopying>
@property (nonatomic, copy, getter=getNewPosition) NSNumber * _Nonnull newPosition MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRSwitchClusterLongPressEvent : NSObject <NSCopying>
@property (nonatomic, copy, getter=getNewPosition) NSNumber * _Nonnull newPosition MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRSwitchClusterShortReleaseEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull previousPosition MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRSwitchClusterLongReleaseEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull previousPosition MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRSwitchClusterMultiPressOngoingEvent : NSObject <NSCopying>
@property (nonatomic, copy, getter=getNewPosition) NSNumber * _Nonnull newPosition MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull currentNumberOfPressesCounted MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRSwitchClusterMultiPressCompleteEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull previousPosition MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy, getter=getNewPosition) NSNumber * _Nonnull newPosition MTR_DEPRECATED("Please use previousPosition", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull totalNumberOfPressesCounted MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTROperationalCredentialsClusterFabricDescriptorStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSData * _Nonnull rootPublicKey MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull vendorID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull vendorId MTR_DEPRECATED("Please use vendorID", ios(16.4, 16.4), macos(13.3, 13.3), watchos(9.4, 9.4), tvos(16.4, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull fabricID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull fabricId MTR_DEPRECATED("Please use fabricID", ios(16.4, 16.4), macos(13.3, 13.3), watchos(9.4, 9.4), tvos(16.4, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull nodeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull nodeId MTR_DEPRECATED("Please use nodeID", ios(16.4, 16.4), macos(13.3, 13.3), watchos(9.4, 9.4), tvos(16.4, 16.4));
@property (nonatomic, copy) NSString * _Nonnull label MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTROperationalCredentialsClusterFabricDescriptorStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTROperationalCredentialsClusterFabricDescriptor : MTROperationalCredentialsClusterFabricDescriptorStruct
@property (nonatomic, copy) NSData * _Nonnull rootPublicKey MTR_DEPRECATED("Please use MTROperationalCredentialsClusterFabricDescriptorStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSString * _Nonnull label MTR_DEPRECATED("Please use MTROperationalCredentialsClusterFabricDescriptorStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_DEPRECATED("Please use MTROperationalCredentialsClusterFabricDescriptorStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROperationalCredentialsClusterNOCStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSData * _Nonnull noc MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSData * _Nullable icac MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupKeyManagementClusterGroupInfoMapStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSArray * _Nonnull endpoints MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSString * _Nullable groupName MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupKeyManagementClusterGroupKeyMapStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull groupKeySetID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupKeyManagementClusterGroupKeySetStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull groupKeySetID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull groupKeySecurityPolicy MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSData * _Nullable epochKey0 MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable epochStartTime0 MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSData * _Nullable epochKey1 MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable epochStartTime1 MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSData * _Nullable epochKey2 MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable epochStartTime2 MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRFixedLabelClusterLabelStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull label MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSString * _Nonnull value MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRUserLabelClusterLabelStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull label MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSString * _Nonnull value MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRBooleanStateClusterStateChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull stateValue MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRICDManagementClusterMonitoringRegistrationStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull checkInNodeID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull monitoredSubject MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTROvenCavityOperationalStateClusterErrorStateStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull errorStateID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable errorStateLabel MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable errorStateDetails MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTROvenCavityOperationalStateClusterOperationalStateStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull operationalStateID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable operationalStateLabel MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTROvenCavityOperationalStateClusterOperationalErrorEvent : NSObject <NSCopying>
@property (nonatomic, copy) MTROvenCavityOperationalStateClusterErrorStateStruct * _Nonnull errorState MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTROvenCavityOperationalStateClusterOperationCompletionEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull completionErrorCode MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable totalOperationalTime MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable pausedTime MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTROvenModeClusterModeTagStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable mfgCode MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull value MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTROvenModeClusterModeOptionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull label MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull mode MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nonnull modeTags MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRModeSelectClusterSemanticTagStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull mfgCode MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull value MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRModeSelectClusterSemanticTagStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRModeSelectClusterSemanticTag : MTRModeSelectClusterSemanticTagStruct
@property (nonatomic, copy) NSNumber * _Nonnull mfgCode MTR_DEPRECATED("Please use MTRModeSelectClusterSemanticTagStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull value MTR_DEPRECATED("Please use MTRModeSelectClusterSemanticTagStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRModeSelectClusterModeOptionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull label MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull mode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSArray * _Nonnull semanticTags MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRLaundryWasherModeClusterModeTagStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable mfgCode MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull value MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRLaundryWasherModeClusterModeOptionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull label MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull mode MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nonnull modeTags MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRRefrigeratorAndTemperatureControlledCabinetModeClusterModeTagStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable mfgCode MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull value MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRRefrigeratorAndTemperatureControlledCabinetModeClusterModeOptionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull label MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull mode MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nonnull modeTags MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4))
@interface MTRRVCRunModeClusterModeTagStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable mfgCode MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@property (nonatomic, copy) NSNumber * _Nonnull value MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@end

MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4))
@interface MTRRVCRunModeClusterModeOptionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull label MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@property (nonatomic, copy) NSNumber * _Nonnull mode MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@property (nonatomic, copy) NSArray * _Nonnull modeTags MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@end

MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4))
@interface MTRRVCCleanModeClusterModeTagStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable mfgCode MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@property (nonatomic, copy) NSNumber * _Nonnull value MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@end

MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4))
@interface MTRRVCCleanModeClusterModeOptionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull label MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@property (nonatomic, copy) NSNumber * _Nonnull mode MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@property (nonatomic, copy) NSArray * _Nonnull modeTags MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRRefrigeratorAlarmClusterNotifyEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull active MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull inactive MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull state MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull mask MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRDishwasherModeClusterModeTagStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable mfgCode MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull value MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRDishwasherModeClusterModeOptionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull label MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull mode MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nonnull modeTags MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRSmokeCOAlarmClusterSmokeAlarmEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull alarmSeverityLevel MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRSmokeCOAlarmClusterCOAlarmEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull alarmSeverityLevel MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRSmokeCOAlarmClusterLowBatteryEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull alarmSeverityLevel MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRSmokeCOAlarmClusterHardwareFaultEvent : NSObject <NSCopying>
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRSmokeCOAlarmClusterEndOfServiceEvent : NSObject <NSCopying>
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRSmokeCOAlarmClusterSelfTestCompleteEvent : NSObject <NSCopying>
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRSmokeCOAlarmClusterAlarmMutedEvent : NSObject <NSCopying>
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRSmokeCOAlarmClusterMuteEndedEvent : NSObject <NSCopying>
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRSmokeCOAlarmClusterInterconnectSmokeAlarmEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull alarmSeverityLevel MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRSmokeCOAlarmClusterInterconnectCOAlarmEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull alarmSeverityLevel MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRSmokeCOAlarmClusterAllClearEvent : NSObject <NSCopying>
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRDishwasherAlarmClusterNotifyEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull active MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull inactive MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull state MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull mask MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRMicrowaveOvenModeClusterModeTagStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable mfgCode MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull value MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRMicrowaveOvenModeClusterModeOptionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull label MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull mode MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nonnull modeTags MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4))
@interface MTROperationalStateClusterErrorStateStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull errorStateID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@property (nonatomic, copy) NSString * _Nullable errorStateLabel MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@property (nonatomic, copy) NSString * _Nullable errorStateDetails MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@end

MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4))
@interface MTROperationalStateClusterOperationalStateStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull operationalStateID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@property (nonatomic, copy) NSString * _Nullable operationalStateLabel MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@end

MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4))
@interface MTROperationalStateClusterOperationalErrorEvent : NSObject <NSCopying>
@property (nonatomic, copy) MTROperationalStateClusterErrorStateStruct * _Nonnull errorState MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@end

MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4))
@interface MTROperationalStateClusterOperationCompletionEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull completionErrorCode MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@property (nonatomic, copy) NSNumber * _Nullable totalOperationalTime MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@property (nonatomic, copy) NSNumber * _Nullable pausedTime MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@end

MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4))
@interface MTRRVCOperationalStateClusterErrorStateStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull errorStateID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@property (nonatomic, copy) NSString * _Nullable errorStateLabel MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@property (nonatomic, copy) NSString * _Nullable errorStateDetails MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@end

MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4))
@interface MTRRVCOperationalStateClusterOperationalStateStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull operationalStateID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@property (nonatomic, copy) NSString * _Nullable operationalStateLabel MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@end

MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4))
@interface MTRRVCOperationalStateClusterOperationalErrorEvent : NSObject <NSCopying>
@property (nonatomic, copy) MTRRVCOperationalStateClusterErrorStateStruct * _Nonnull errorState MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@end

MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4))
@interface MTRRVCOperationalStateClusterOperationCompletionEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull completionErrorCode MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@property (nonatomic, copy) NSNumber * _Nullable totalOperationalTime MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@property (nonatomic, copy) NSNumber * _Nullable pausedTime MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRHEPAFilterMonitoringClusterReplacementProductStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull productIdentifierType MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nonnull productIdentifierValue MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRActivatedCarbonFilterMonitoringClusterReplacementProductStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull productIdentifierType MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nonnull productIdentifierValue MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRBooleanStateConfigurationClusterAlarmsStateChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull alarmsActive MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable alarmsSuppressed MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRBooleanStateConfigurationClusterSensorFaultEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull sensorFault MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRValveConfigurationAndControlClusterValveStateChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull valveState MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable valveLevel MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRValveConfigurationAndControlClusterValveFaultEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull valveFault MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRElectricalEnergyMeasurementClusterMeasurementAccuracyRangeStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull rangeMin MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull rangeMax MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable percentMax MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable percentMin MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable percentTypical MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable fixedMax MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable fixedMin MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable fixedTypical MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRElectricalEnergyMeasurementClusterMeasurementAccuracyStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull measurementType MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull measured MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull minMeasuredValue MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull maxMeasuredValue MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nonnull accuracyRanges MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRElectricalEnergyMeasurementClusterEnergyMeasurementStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull energy MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable startTimestamp MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable endTimestamp MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable startSystime MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable endSystime MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRElectricalEnergyMeasurementClusterCumulativeEnergyMeasuredEvent : NSObject <NSCopying>
@property (nonatomic, copy) MTRElectricalEnergyMeasurementClusterEnergyMeasurementStruct * _Nullable energyImported MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRElectricalEnergyMeasurementClusterEnergyMeasurementStruct * _Nullable energyExported MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRElectricalEnergyMeasurementClusterPeriodicEnergyMeasuredEvent : NSObject <NSCopying>
@property (nonatomic, copy) MTRElectricalEnergyMeasurementClusterEnergyMeasurementStruct * _Nullable energyImported MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRElectricalEnergyMeasurementClusterEnergyMeasurementStruct * _Nullable energyExported MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRDemandResponseLoadControlClusterHeatingSourceControlStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull heatingSource MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRDemandResponseLoadControlClusterPowerSavingsControlStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull powerSavings MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRDemandResponseLoadControlClusterDutyCycleControlStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull dutyCycle MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRDemandResponseLoadControlClusterAverageLoadControlStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull loadAdjustment MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRDemandResponseLoadControlClusterTemperatureControlStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable coolingTempOffset MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable heatingtTempOffset MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable coolingTempSetpoint MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable heatingTempSetpoint MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRDemandResponseLoadControlClusterLoadControlEventTransitionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull duration MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull control MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRDemandResponseLoadControlClusterTemperatureControlStruct * _Nullable temperatureControl MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRDemandResponseLoadControlClusterAverageLoadControlStruct * _Nullable averageLoadControl MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRDemandResponseLoadControlClusterDutyCycleControlStruct * _Nullable dutyCycleControl MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRDemandResponseLoadControlClusterPowerSavingsControlStruct * _Nullable powerSavingsControl MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRDemandResponseLoadControlClusterHeatingSourceControlStruct * _Nullable heatingSourceControl MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRDemandResponseLoadControlClusterLoadControlEventStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSData * _Nonnull eventID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSData * _Nullable programID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull control MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull deviceClass MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable enrollmentGroup MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull criticality MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable startTime MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nonnull transitions MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRDemandResponseLoadControlClusterLoadControlProgramStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSData * _Nonnull programID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nonnull name MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable enrollmentGroup MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable randomStartMinutes MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable randomDurationMinutes MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRDemandResponseLoadControlClusterLoadControlEventStatusChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSData * _Nonnull eventID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable transitionIndex MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull status MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull criticality MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull control MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRDemandResponseLoadControlClusterTemperatureControlStruct * _Nullable temperatureControl MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRDemandResponseLoadControlClusterAverageLoadControlStruct * _Nullable averageLoadControl MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRDemandResponseLoadControlClusterDutyCycleControlStruct * _Nullable dutyCycleControl MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRDemandResponseLoadControlClusterPowerSavingsControlStruct * _Nullable powerSavingsControl MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRDemandResponseLoadControlClusterHeatingSourceControlStruct * _Nullable heatingSourceControl MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRDeviceEnergyManagementClusterCostStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull costType MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull value MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull decimalPoints MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable currency MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRDeviceEnergyManagementClusterSlotStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull minDuration MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull maxDuration MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull defaultDuration MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull elapsedSlotTime MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull remainingSlotTime MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull slotIsPauseable MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull minPauseDuration MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull maxPauseDuration MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable manufacturerESAState MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable nominalPower MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable minPower MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable maxPower MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable nominalEnergy MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nullable costs MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable minPowerAdjustment MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable maxPowerAdjustment MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable minDurationAdjustment MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable maxDurationAdjustment MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRDeviceEnergyManagementClusterForecastStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull forecastId MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable activeSlotNumber MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull startTime MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull endTime MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable earliestStartTime MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable latestEndTime MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull isPauseable MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nonnull slots MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRDeviceEnergyManagementClusterConstraintsStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull startTime MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull duration MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable nominalPower MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable maximumEnergy MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable loadControl MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRDeviceEnergyManagementClusterPowerAdjustStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull minPower MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull maxPower MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull minDuration MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull maxDuration MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRDeviceEnergyManagementClusterSlotAdjustmentStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull slotIndex MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull nominalPower MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull duration MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRDeviceEnergyManagementClusterPowerAdjustStartEvent : NSObject <NSCopying>
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRDeviceEnergyManagementClusterPowerAdjustEndEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull cause MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull duration MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull energyUse MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRDeviceEnergyManagementClusterPausedEvent : NSObject <NSCopying>
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRDeviceEnergyManagementClusterResumedEvent : NSObject <NSCopying>
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTREnergyEVSEClusterChargingTargetStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull targetTimeMinutesPastMidnight MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable targetSoC MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable addedEnergy MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTREnergyEVSEClusterEVConnectedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull sessionID MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTREnergyEVSEClusterEVNotDetectedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull sessionID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull state MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull sessionDuration MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull sessionEnergyCharged MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable sessionEnergyDischarged MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTREnergyEVSEClusterEnergyTransferStartedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull sessionID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull state MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull maximumCurrent MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTREnergyEVSEClusterEnergyTransferStoppedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull sessionID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull state MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull reason MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull energyTransferred MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTREnergyEVSEClusterFaultEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable sessionID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull state MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull faultStatePreviousState MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull faultStateCurrentState MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTREnergyEVSEClusterRFIDEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSData * _Nonnull uid MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTREnergyPreferenceClusterBalanceStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull step MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable label MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRDoorLockClusterCredentialStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull credentialType MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull credentialIndex MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRDoorLockClusterCredentialStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRDoorLockClusterDlCredential : MTRDoorLockClusterCredentialStruct
@property (nonatomic, copy) NSNumber * _Nonnull credentialType MTR_DEPRECATED("Please use MTRDoorLockClusterCredentialStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull credentialIndex MTR_DEPRECATED("Please use MTRDoorLockClusterCredentialStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterDoorLockAlarmEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull alarmCode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterDoorStateChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull doorState MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterLockOperationEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull lockOperationType MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull operationSource MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable userIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable fabricIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable sourceNode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSArray * _Nullable credentials MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterLockOperationErrorEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull lockOperationType MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull operationSource MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull operationError MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable userIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable fabricIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable sourceNode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSArray * _Nullable credentials MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterLockUserChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull lockDataType MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull dataOperationType MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nonnull operationSource MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable userIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable fabricIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable sourceNode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@property (nonatomic, copy) NSNumber * _Nullable dataIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterSupplyVoltageLowEvent : NSObject <NSCopying>
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterSupplyVoltageHighEvent : NSObject <NSCopying>
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterPowerMissingPhaseEvent : NSObject <NSCopying>
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterSystemPressureLowEvent : NSObject <NSCopying>
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterSystemPressureHighEvent : NSObject <NSCopying>
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterDryRunningEvent : NSObject <NSCopying>
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterMotorTemperatureHighEvent : NSObject <NSCopying>
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterPumpMotorFatalFailureEvent : NSObject <NSCopying>
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterElectronicTemperatureHighEvent : NSObject <NSCopying>
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterPumpBlockedEvent : NSObject <NSCopying>
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterSensorFailureEvent : NSObject <NSCopying>
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterElectronicNonFatalFailureEvent : NSObject <NSCopying>
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterElectronicFatalFailureEvent : NSObject <NSCopying>
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterGeneralFaultEvent : NSObject <NSCopying>
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterLeakageEvent : NSObject <NSCopying>
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterAirDetectionEvent : NSObject <NSCopying>
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRPumpConfigurationAndControlClusterTurbineOperationEvent : NSObject <NSCopying>
@end

MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4))
@interface MTRThermostatClusterWeeklyScheduleTransitionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull transitionTime MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@property (nonatomic, copy) NSNumber * _Nullable heatSetpoint MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@property (nonatomic, copy) NSNumber * _Nullable coolSetpoint MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
@end

MTR_DEPRECATED("Please use MTRThermostatClusterWeeklyScheduleTransitionStruct", ios(16.1, 17.4), macos(13.0, 14.4), watchos(9.1, 10.4), tvos(16.1, 17.4))
@interface MTRThermostatClusterThermostatScheduleTransition : MTRThermostatClusterWeeklyScheduleTransitionStruct
@property (nonatomic, copy) NSNumber * _Nonnull transitionTime MTR_DEPRECATED("Please use MTRThermostatClusterWeeklyScheduleTransitionStruct", ios(16.1, 17.4), macos(13.0, 14.4), watchos(9.1, 10.4), tvos(16.1, 17.4));
@property (nonatomic, copy) NSNumber * _Nullable heatSetpoint MTR_DEPRECATED("Please use MTRThermostatClusterWeeklyScheduleTransitionStruct", ios(16.1, 17.4), macos(13.0, 14.4), watchos(9.1, 10.4), tvos(16.1, 17.4));
@property (nonatomic, copy) NSNumber * _Nullable coolSetpoint MTR_DEPRECATED("Please use MTRThermostatClusterWeeklyScheduleTransitionStruct", ios(16.1, 17.4), macos(13.0, 14.4), watchos(9.1, 10.4), tvos(16.1, 17.4));
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRChannelClusterProgramCastStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull name MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nonnull role MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRChannelClusterProgramCategoryStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull category MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable subCategory MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRChannelClusterSeriesInfoStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull season MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nonnull episode MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRChannelClusterChannelInfoStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull majorNumber MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull minorNumber MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nullable name MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nullable callSign MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nullable affiliateCallSign MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nullable identifier MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable type MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_DEPRECATED("Please use MTRChannelClusterChannelInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRChannelClusterChannelInfo : MTRChannelClusterChannelInfoStruct
@property (nonatomic, copy) NSNumber * _Nonnull majorNumber MTR_DEPRECATED("Please use MTRChannelClusterChannelInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull minorNumber MTR_DEPRECATED("Please use MTRChannelClusterChannelInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSString * _Nullable name MTR_DEPRECATED("Please use MTRChannelClusterChannelInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSString * _Nullable callSign MTR_DEPRECATED("Please use MTRChannelClusterChannelInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSString * _Nullable affiliateCallSign MTR_DEPRECATED("Please use MTRChannelClusterChannelInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRChannelClusterProgramStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull identifier MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRChannelClusterChannelInfoStruct * _Nonnull channel MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull startTime MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull endTime MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nonnull title MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable subtitle MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable descriptionString MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nullable audioLanguages MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nullable ratings MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable thumbnailUrl MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable posterArtUrl MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable dvbiUrl MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable releaseDate MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable parentalGuidanceText MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable recordingFlag MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRChannelClusterSeriesInfoStruct * _Nullable seriesInfo MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nullable categoryList MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nullable castList MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nullable externalIDList MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRChannelClusterPageTokenStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable limit MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable after MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable before MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRChannelClusterChannelPagingStruct : NSObject <NSCopying>
@property (nonatomic, copy) MTRChannelClusterPageTokenStruct * _Nullable previousToken MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRChannelClusterPageTokenStruct * _Nullable nextToken MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRChannelClusterAdditionalInfoStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull name MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nonnull value MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRChannelClusterLineupInfoStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull operatorName MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nullable lineupName MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nullable postalCode MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull lineupInfoType MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRChannelClusterLineupInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRChannelClusterLineupInfo : MTRChannelClusterLineupInfoStruct
@property (nonatomic, copy) NSString * _Nonnull operatorName MTR_DEPRECATED("Please use MTRChannelClusterLineupInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSString * _Nullable lineupName MTR_DEPRECATED("Please use MTRChannelClusterLineupInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSString * _Nullable postalCode MTR_DEPRECATED("Please use MTRChannelClusterLineupInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull lineupInfoType MTR_DEPRECATED("Please use MTRChannelClusterLineupInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRTargetNavigatorClusterTargetInfoStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull identifier MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nonnull name MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRTargetNavigatorClusterTargetInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTargetNavigatorClusterTargetInfo : MTRTargetNavigatorClusterTargetInfoStruct
@property (nonatomic, copy) NSNumber * _Nonnull identifier MTR_DEPRECATED("Please use MTRTargetNavigatorClusterTargetInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSString * _Nonnull name MTR_DEPRECATED("Please use MTRTargetNavigatorClusterTargetInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRTargetNavigatorClusterTargetUpdatedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull targetList MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull currentTarget MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSData * _Nonnull data MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRMediaPlaybackClusterTrackAttributesStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull languageCode MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable displayName MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRMediaPlaybackClusterTrackStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull id MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRMediaPlaybackClusterTrackAttributesStruct * _Nullable trackAttributes MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRMediaPlaybackClusterPlaybackPositionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull updatedAt MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nullable position MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRMediaPlaybackClusterPlaybackPositionStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRMediaPlaybackClusterPlaybackPosition : MTRMediaPlaybackClusterPlaybackPositionStruct
@property (nonatomic, copy) NSNumber * _Nonnull updatedAt MTR_DEPRECATED("Please use MTRMediaPlaybackClusterPlaybackPositionStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nullable position MTR_DEPRECATED("Please use MTRMediaPlaybackClusterPlaybackPositionStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRMediaPlaybackClusterStateChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull currentState MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull startTime MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull duration MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRMediaPlaybackClusterPlaybackPositionStruct * _Nonnull sampledPosition MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull playbackSpeed MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull seekRangeEnd MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull seekRangeStart MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSData * _Nullable data MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull audioAdvanceUnmuted MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRMediaInputClusterInputInfoStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull index MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull inputType MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nonnull name MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nonnull descriptionString MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRMediaInputClusterInputInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRMediaInputClusterInputInfo : MTRMediaInputClusterInputInfoStruct
@property (nonatomic, copy) NSNumber * _Nonnull index MTR_DEPRECATED("Please use MTRMediaInputClusterInputInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull inputType MTR_DEPRECATED("Please use MTRMediaInputClusterInputInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSString * _Nonnull name MTR_DEPRECATED("Please use MTRMediaInputClusterInputInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSString * _Nonnull descriptionString MTR_DEPRECATED("Please use MTRMediaInputClusterInputInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRContentLauncherClusterDimensionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull width MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull height MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull metric MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRContentLauncherClusterDimensionStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRContentLauncherClusterDimension : MTRContentLauncherClusterDimensionStruct
@property (nonatomic, copy) NSNumber * _Nonnull width MTR_DEPRECATED("Please use MTRContentLauncherClusterDimensionStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull height MTR_DEPRECATED("Please use MTRContentLauncherClusterDimensionStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull metric MTR_DEPRECATED("Please use MTRContentLauncherClusterDimensionStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRContentLauncherClusterTrackPreferenceStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull languageCode MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nullable characteristics MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull audioOutputIndex MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRContentLauncherClusterPlaybackPreferencesStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull playbackPosition MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRContentLauncherClusterTrackPreferenceStruct * _Nonnull textTrack MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nullable audioTracks MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRContentLauncherClusterAdditionalInfoStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull name MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nonnull value MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRContentLauncherClusterAdditionalInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRContentLauncherClusterAdditionalInfo : MTRContentLauncherClusterAdditionalInfoStruct
@property (nonatomic, copy) NSString * _Nonnull name MTR_DEPRECATED("Please use MTRContentLauncherClusterAdditionalInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSString * _Nonnull value MTR_DEPRECATED("Please use MTRContentLauncherClusterAdditionalInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRContentLauncherClusterParameterStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull type MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nonnull value MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nullable externalIDList MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRContentLauncherClusterParameterStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRContentLauncherClusterParameter : MTRContentLauncherClusterParameterStruct
@property (nonatomic, copy) NSNumber * _Nonnull type MTR_DEPRECATED("Please use MTRContentLauncherClusterParameterStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSString * _Nonnull value MTR_DEPRECATED("Please use MTRContentLauncherClusterParameterStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSArray * _Nullable externalIDList MTR_DEPRECATED("Please use MTRContentLauncherClusterParameterStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRContentLauncherClusterContentSearchStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull parameterList MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRContentLauncherClusterContentSearchStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRContentLauncherClusterContentSearch : MTRContentLauncherClusterContentSearchStruct
@property (nonatomic, copy) NSArray * _Nonnull parameterList MTR_DEPRECATED("Please use MTRContentLauncherClusterContentSearchStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRContentLauncherClusterStyleInformationStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nullable imageURL MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nullable imageUrl MTR_DEPRECATED("Please use imageURL", ios(16.4, 16.4), macos(13.3, 13.3), watchos(9.4, 9.4), tvos(16.4, 16.4));
@property (nonatomic, copy) NSString * _Nullable color MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) MTRContentLauncherClusterDimensionStruct * _Nullable size MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRContentLauncherClusterStyleInformationStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRContentLauncherClusterStyleInformation : MTRContentLauncherClusterStyleInformationStruct
@property (nonatomic, copy) NSString * _Nullable color MTR_DEPRECATED("Please use MTRContentLauncherClusterStyleInformationStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) MTRContentLauncherClusterDimensionStruct * _Nullable size MTR_DEPRECATED("Please use MTRContentLauncherClusterStyleInformationStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRContentLauncherClusterBrandingInformationStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull providerName MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) MTRContentLauncherClusterStyleInformationStruct * _Nullable background MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) MTRContentLauncherClusterStyleInformationStruct * _Nullable logo MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) MTRContentLauncherClusterStyleInformationStruct * _Nullable progressBar MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) MTRContentLauncherClusterStyleInformationStruct * _Nullable splash MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) MTRContentLauncherClusterStyleInformationStruct * _Nullable waterMark MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRContentLauncherClusterBrandingInformationStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRContentLauncherClusterBrandingInformation : MTRContentLauncherClusterBrandingInformationStruct
@property (nonatomic, copy) NSString * _Nonnull providerName MTR_DEPRECATED("Please use MTRContentLauncherClusterBrandingInformationStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) MTRContentLauncherClusterStyleInformationStruct * _Nullable background MTR_DEPRECATED("Please use MTRContentLauncherClusterBrandingInformationStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) MTRContentLauncherClusterStyleInformationStruct * _Nullable logo MTR_DEPRECATED("Please use MTRContentLauncherClusterBrandingInformationStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) MTRContentLauncherClusterStyleInformationStruct * _Nullable progressBar MTR_DEPRECATED("Please use MTRContentLauncherClusterBrandingInformationStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) MTRContentLauncherClusterStyleInformationStruct * _Nullable splash MTR_DEPRECATED("Please use MTRContentLauncherClusterBrandingInformationStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) MTRContentLauncherClusterStyleInformationStruct * _Nullable waterMark MTR_DEPRECATED("Please use MTRContentLauncherClusterBrandingInformationStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRAudioOutputClusterOutputInfoStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull index MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull outputType MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nonnull name MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRAudioOutputClusterOutputInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRAudioOutputClusterOutputInfo : MTRAudioOutputClusterOutputInfoStruct
@property (nonatomic, copy) NSNumber * _Nonnull index MTR_DEPRECATED("Please use MTRAudioOutputClusterOutputInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull outputType MTR_DEPRECATED("Please use MTRAudioOutputClusterOutputInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSString * _Nonnull name MTR_DEPRECATED("Please use MTRAudioOutputClusterOutputInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRApplicationLauncherClusterApplicationStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull catalogVendorID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull catalogVendorId MTR_DEPRECATED("Please use catalogVendorID", ios(16.4, 16.4), macos(13.3, 13.3), watchos(9.4, 9.4), tvos(16.4, 16.4));
@property (nonatomic, copy) NSString * _Nonnull applicationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nonnull applicationId MTR_DEPRECATED("Please use applicationID", ios(16.4, 16.4), macos(13.3, 13.3), watchos(9.4, 9.4), tvos(16.4, 16.4));
@end

MTR_DEPRECATED("Please use MTRApplicationLauncherClusterApplicationStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRApplicationLauncherClusterApplication : MTRApplicationLauncherClusterApplicationStruct
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRApplicationLauncherClusterApplicationEPStruct : NSObject <NSCopying>
@property (nonatomic, copy) MTRApplicationLauncherClusterApplicationStruct * _Nonnull application MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nullable endpoint MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRApplicationLauncherClusterApplicationEPStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRApplicationLauncherClusterApplicationEP : MTRApplicationLauncherClusterApplicationEPStruct
@property (nonatomic, copy) MTRApplicationLauncherClusterApplicationStruct * _Nonnull application MTR_DEPRECATED("Please use MTRApplicationLauncherClusterApplicationEPStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nullable endpoint MTR_DEPRECATED("Please use MTRApplicationLauncherClusterApplicationEPStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRApplicationBasicClusterApplicationStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull catalogVendorID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull catalogVendorId MTR_DEPRECATED("Please use catalogVendorID", ios(16.4, 16.4), macos(13.3, 13.3), watchos(9.4, 9.4), tvos(16.4, 16.4));
@property (nonatomic, copy) NSString * _Nonnull applicationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nonnull applicationId MTR_DEPRECATED("Please use applicationID", ios(16.4, 16.4), macos(13.3, 13.3), watchos(9.4, 9.4), tvos(16.4, 16.4));
@end

MTR_DEPRECATED("Please use MTRApplicationBasicClusterApplicationStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRApplicationBasicClusterApplicationBasicApplication : MTRApplicationBasicClusterApplicationStruct
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRAccountLoginClusterLoggedOutEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable node MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRContentControlClusterRatingNameStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull ratingName MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable ratingNameDesc MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRContentControlClusterRemainingScreenTimeExpiredEvent : NSObject <NSCopying>
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterSimpleStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull a MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull b MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull c MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSData * _Nonnull d MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nonnull e MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull f MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull g MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull h MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterSimpleStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterSimpleStruct : MTRUnitTestingClusterSimpleStruct
@property (nonatomic, copy) NSNumber * _Nonnull a MTR_DEPRECATED("Please use MTRUnitTestingClusterSimpleStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull b MTR_DEPRECATED("Please use MTRUnitTestingClusterSimpleStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull c MTR_DEPRECATED("Please use MTRUnitTestingClusterSimpleStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSData * _Nonnull d MTR_DEPRECATED("Please use MTRUnitTestingClusterSimpleStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSString * _Nonnull e MTR_DEPRECATED("Please use MTRUnitTestingClusterSimpleStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull f MTR_DEPRECATED("Please use MTRUnitTestingClusterSimpleStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull g MTR_DEPRECATED("Please use MTRUnitTestingClusterSimpleStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull h MTR_DEPRECATED("Please use MTRUnitTestingClusterSimpleStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestFabricScoped : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull fabricSensitiveInt8u MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nullable optionalFabricSensitiveInt8u MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nullable nullableFabricSensitiveInt8u MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalFabricSensitiveInt8u MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nonnull fabricSensitiveCharString MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nonnull fabricSensitiveStruct MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nonnull fabricSensitiveInt8uList MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestFabricScoped", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestFabricScoped : MTRUnitTestingClusterTestFabricScoped
@property (nonatomic, copy) NSNumber * _Nonnull fabricSensitiveInt8u MTR_DEPRECATED("Please use MTRUnitTestingClusterTestFabricScoped", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nullable optionalFabricSensitiveInt8u MTR_DEPRECATED("Please use MTRUnitTestingClusterTestFabricScoped", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nullable nullableFabricSensitiveInt8u MTR_DEPRECATED("Please use MTRUnitTestingClusterTestFabricScoped", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalFabricSensitiveInt8u MTR_DEPRECATED("Please use MTRUnitTestingClusterTestFabricScoped", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSString * _Nonnull fabricSensitiveCharString MTR_DEPRECATED("Please use MTRUnitTestingClusterTestFabricScoped", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) MTRTestClusterClusterSimpleStruct * _Nonnull fabricSensitiveStruct MTR_DEPRECATED("Please use MTRUnitTestingClusterTestFabricScoped", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSArray * _Nonnull fabricSensitiveInt8uList MTR_DEPRECATED("Please use MTRUnitTestingClusterTestFabricScoped", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_DEPRECATED("Please use MTRUnitTestingClusterTestFabricScoped", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterNullablesAndOptionalsStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable nullableInt MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nullable optionalInt MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalInt MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nullable nullableString MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nullable optionalString MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nullable nullableOptionalString MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable nullableStruct MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable optionalStruct MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable nullableOptionalStruct MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nullable nullableList MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nullable optionalList MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nullable nullableOptionalList MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterNullablesAndOptionalsStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterNullablesAndOptionalsStruct : MTRUnitTestingClusterNullablesAndOptionalsStruct
@property (nonatomic, copy) NSNumber * _Nullable nullableInt MTR_DEPRECATED("Please use MTRUnitTestingClusterNullablesAndOptionalsStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nullable optionalInt MTR_DEPRECATED("Please use MTRUnitTestingClusterNullablesAndOptionalsStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalInt MTR_DEPRECATED("Please use MTRUnitTestingClusterNullablesAndOptionalsStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSString * _Nullable nullableString MTR_DEPRECATED("Please use MTRUnitTestingClusterNullablesAndOptionalsStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSString * _Nullable optionalString MTR_DEPRECATED("Please use MTRUnitTestingClusterNullablesAndOptionalsStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSString * _Nullable nullableOptionalString MTR_DEPRECATED("Please use MTRUnitTestingClusterNullablesAndOptionalsStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) MTRTestClusterClusterSimpleStruct * _Nullable nullableStruct MTR_DEPRECATED("Please use MTRUnitTestingClusterNullablesAndOptionalsStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) MTRTestClusterClusterSimpleStruct * _Nullable optionalStruct MTR_DEPRECATED("Please use MTRUnitTestingClusterNullablesAndOptionalsStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) MTRTestClusterClusterSimpleStruct * _Nullable nullableOptionalStruct MTR_DEPRECATED("Please use MTRUnitTestingClusterNullablesAndOptionalsStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSArray * _Nullable nullableList MTR_DEPRECATED("Please use MTRUnitTestingClusterNullablesAndOptionalsStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSArray * _Nullable optionalList MTR_DEPRECATED("Please use MTRUnitTestingClusterNullablesAndOptionalsStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSArray * _Nullable nullableOptionalList MTR_DEPRECATED("Please use MTRUnitTestingClusterNullablesAndOptionalsStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterNestedStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull a MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull b MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nonnull c MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterNestedStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterNestedStruct : MTRUnitTestingClusterNestedStruct
@property (nonatomic, copy) NSNumber * _Nonnull a MTR_DEPRECATED("Please use MTRUnitTestingClusterNestedStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull b MTR_DEPRECATED("Please use MTRUnitTestingClusterNestedStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) MTRTestClusterClusterSimpleStruct * _Nonnull c MTR_DEPRECATED("Please use MTRUnitTestingClusterNestedStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterNestedStructList : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull a MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull b MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nonnull c MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nonnull d MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nonnull e MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nonnull f MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nonnull g MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterNestedStructList", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterNestedStructList : MTRUnitTestingClusterNestedStructList
@property (nonatomic, copy) NSNumber * _Nonnull a MTR_DEPRECATED("Please use MTRUnitTestingClusterNestedStructList", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull b MTR_DEPRECATED("Please use MTRUnitTestingClusterNestedStructList", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) MTRTestClusterClusterSimpleStruct * _Nonnull c MTR_DEPRECATED("Please use MTRUnitTestingClusterNestedStructList", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSArray * _Nonnull d MTR_DEPRECATED("Please use MTRUnitTestingClusterNestedStructList", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSArray * _Nonnull e MTR_DEPRECATED("Please use MTRUnitTestingClusterNestedStructList", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSArray * _Nonnull f MTR_DEPRECATED("Please use MTRUnitTestingClusterNestedStructList", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSArray * _Nonnull g MTR_DEPRECATED("Please use MTRUnitTestingClusterNestedStructList", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterDoubleNestedStructList : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull a MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterDoubleNestedStructList", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterDoubleNestedStructList : MTRUnitTestingClusterDoubleNestedStructList
@property (nonatomic, copy) NSArray * _Nonnull a MTR_DEPRECATED("Please use MTRUnitTestingClusterDoubleNestedStructList", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestListStructOctet : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull member1 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSData * _Nonnull member2 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestListStructOctet", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestListStructOctet : MTRUnitTestingClusterTestListStructOctet
@property (nonatomic, copy) NSNumber * _Nonnull member1 MTR_DEPRECATED("Please use MTRUnitTestingClusterTestListStructOctet", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSData * _Nonnull member2 MTR_DEPRECATED("Please use MTRUnitTestingClusterTestListStructOctet", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestEventEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull arg1 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull arg2 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull arg3 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nonnull arg4 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nonnull arg5 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSArray * _Nonnull arg6 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestEventEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestEventEvent : MTRUnitTestingClusterTestEventEvent
@property (nonatomic, copy) NSNumber * _Nonnull arg1 MTR_DEPRECATED("Please use MTRUnitTestingClusterTestEventEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull arg2 MTR_DEPRECATED("Please use MTRUnitTestingClusterTestEventEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull arg3 MTR_DEPRECATED("Please use MTRUnitTestingClusterTestEventEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) MTRTestClusterClusterSimpleStruct * _Nonnull arg4 MTR_DEPRECATED("Please use MTRUnitTestingClusterTestEventEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSArray * _Nonnull arg5 MTR_DEPRECATED("Please use MTRUnitTestingClusterTestEventEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSArray * _Nonnull arg6 MTR_DEPRECATED("Please use MTRUnitTestingClusterTestEventEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestFabricScopedEventEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestFabricScopedEventEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestFabricScopedEventEvent : MTRUnitTestingClusterTestFabricScopedEventEvent
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_DEPRECATED("Please use MTRUnitTestingClusterTestFabricScopedEventEvent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRSampleMEIClusterPingCountEventEvent : NSObject <NSCopying>
@property (nonatomic, copy, getter=getCount) NSNumber * _Nonnull count MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_PROVISIONALLY_AVAILABLE;
@end

NS_ASSUME_NONNULL_END
