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

MTR_PROVISIONALLY_AVAILABLE
@interface MTRDataTypeTestGlobalStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull name MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable myBitmap MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable myEnum MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRDataTypeLocationDescriptorStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull locationName MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable floorNumber MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable areaType MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRDataTypeAtomicAttributeStatusStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull attributeID MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull statusCode MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
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

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRAccessControlClusterAccessRestrictionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull type MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable id MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRAccessControlClusterCommissioningAccessRestrictionEntryStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull endpoint MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull cluster MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSArray * _Nonnull restrictions MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRAccessControlClusterAccessRestrictionEntryStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull endpoint MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull cluster MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSArray * _Nonnull restrictions MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
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

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRAccessControlClusterFabricRestrictionReviewUpdateEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull token MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSString * _Nullable instruction MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSString * _Nullable arlRequestFlowUrl MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
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

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRTimeSynchronizationClusterFabricScopedTrustedTimeSourceStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull nodeID MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull endpoint MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
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

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRTimeSynchronizationClusterTrustedTimeSourceStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull nodeID MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull endpoint MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRTimeSynchronizationClusterDSTTableEmptyEvent : NSObject <NSCopying>
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRTimeSynchronizationClusterDSTStatusEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull dstOffsetActive MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRTimeSynchronizationClusterTimeZoneStatusEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull offset MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSString * _Nullable name MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRTimeSynchronizationClusterTimeFailureEvent : NSObject <NSCopying>
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
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

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRBridgedDeviceBasicInformationClusterActiveChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull promisedActiveDuration MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
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

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRICDManagementClusterMonitoringRegistrationStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull checkInNodeID MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull monitoredSubject MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull clientType MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTROvenCavityOperationalStateClusterErrorStateStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull errorStateID MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSString * _Nullable errorStateLabel MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSString * _Nullable errorStateDetails MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTROvenCavityOperationalStateClusterOperationalStateStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull operationalStateID MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSString * _Nullable operationalStateLabel MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTROvenCavityOperationalStateClusterOperationalErrorEvent : NSObject <NSCopying>
@property (nonatomic, copy) MTROvenCavityOperationalStateClusterErrorStateStruct * _Nonnull errorState MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTROvenCavityOperationalStateClusterOperationCompletionEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull completionErrorCode MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable totalOperationalTime MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable pausedTime MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTROvenModeClusterModeTagStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable mfgCode MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull value MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTROvenModeClusterModeOptionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull label MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull mode MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSArray * _Nonnull modeTags MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
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

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRLaundryWasherModeClusterModeTagStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable mfgCode MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull value MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRLaundryWasherModeClusterModeOptionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull label MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull mode MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSArray * _Nonnull modeTags MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRRefrigeratorAndTemperatureControlledCabinetModeClusterModeTagStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable mfgCode MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull value MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRRefrigeratorAndTemperatureControlledCabinetModeClusterModeOptionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull label MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull mode MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSArray * _Nonnull modeTags MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
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

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRRefrigeratorAlarmClusterNotifyEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull active MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull inactive MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull state MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull mask MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRDishwasherModeClusterModeTagStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable mfgCode MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull value MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRDishwasherModeClusterModeOptionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull label MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull mode MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSArray * _Nonnull modeTags MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRSmokeCOAlarmClusterSmokeAlarmEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull alarmSeverityLevel MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@end

MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRSmokeCOAlarmClusterCOAlarmEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull alarmSeverityLevel MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@end

MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRSmokeCOAlarmClusterLowBatteryEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull alarmSeverityLevel MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@end

MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRSmokeCOAlarmClusterHardwareFaultEvent : NSObject <NSCopying>
@end

MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRSmokeCOAlarmClusterEndOfServiceEvent : NSObject <NSCopying>
@end

MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRSmokeCOAlarmClusterSelfTestCompleteEvent : NSObject <NSCopying>
@end

MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRSmokeCOAlarmClusterAlarmMutedEvent : NSObject <NSCopying>
@end

MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRSmokeCOAlarmClusterMuteEndedEvent : NSObject <NSCopying>
@end

MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRSmokeCOAlarmClusterInterconnectSmokeAlarmEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull alarmSeverityLevel MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@end

MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRSmokeCOAlarmClusterInterconnectCOAlarmEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull alarmSeverityLevel MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@end

MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRSmokeCOAlarmClusterAllClearEvent : NSObject <NSCopying>
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRDishwasherAlarmClusterNotifyEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull active MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull inactive MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull state MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull mask MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRMicrowaveOvenModeClusterModeTagStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable mfgCode MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull value MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRMicrowaveOvenModeClusterModeOptionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull label MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull mode MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSArray * _Nonnull modeTags MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
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
@interface MTRScenesManagementClusterAttributeValuePairStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull attributeID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable valueUnsigned8 MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable valueSigned8 MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable valueUnsigned16 MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable valueSigned16 MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable valueUnsigned32 MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable valueSigned32 MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable valueUnsigned64 MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable valueSigned64 MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRScenesManagementClusterExtensionFieldSet : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull clusterID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nonnull attributeValueList MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRScenesManagementClusterSceneInfoStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull sceneCount MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull currentScene MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull currentGroup MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull sceneValid MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull remainingCapacity MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRHEPAFilterMonitoringClusterReplacementProductStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull productIdentifierType MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSString * _Nonnull productIdentifierValue MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@end

MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRActivatedCarbonFilterMonitoringClusterReplacementProductStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull productIdentifierType MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSString * _Nonnull productIdentifierValue MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@end

MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRBooleanStateConfigurationClusterAlarmsStateChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull alarmsActive MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nullable alarmsSuppressed MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@end

MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRBooleanStateConfigurationClusterSensorFaultEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull sensorFault MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@end

MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRValveConfigurationAndControlClusterValveStateChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull valveState MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nullable valveLevel MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@end

MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRValveConfigurationAndControlClusterValveFaultEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull valveFault MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@end

MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRElectricalPowerMeasurementClusterMeasurementAccuracyRangeStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull rangeMin MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nonnull rangeMax MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nullable percentMax MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nullable percentMin MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nullable percentTypical MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nullable fixedMax MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nullable fixedMin MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nullable fixedTypical MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@end

MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRElectricalPowerMeasurementClusterMeasurementAccuracyStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull measurementType MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nonnull measured MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nonnull minMeasuredValue MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nonnull maxMeasuredValue MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSArray * _Nonnull accuracyRanges MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@end

MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRElectricalPowerMeasurementClusterHarmonicMeasurementStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull order MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nullable measurement MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@end

MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRElectricalPowerMeasurementClusterMeasurementRangeStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull measurementType MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nonnull min MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nonnull max MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nullable startTimestamp MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nullable endTimestamp MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nullable minTimestamp MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nullable maxTimestamp MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nullable startSystime MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nullable endSystime MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nullable minSystime MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nullable maxSystime MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@end

MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRElectricalPowerMeasurementClusterMeasurementPeriodRangesEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull ranges MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRElectricalEnergyMeasurementClusterMeasurementAccuracyRangeStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull rangeMin MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull rangeMax MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable percentMax MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable percentMin MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable percentTypical MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable fixedMax MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable fixedMin MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable fixedTypical MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRElectricalEnergyMeasurementClusterMeasurementAccuracyStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull measurementType MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nonnull measured MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nonnull minMeasuredValue MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nonnull maxMeasuredValue MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSArray * _Nonnull accuracyRanges MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@end

MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRElectricalEnergyMeasurementClusterCumulativeEnergyResetStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable importedResetTimestamp MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nullable exportedResetTimestamp MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nullable importedResetSystime MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nullable exportedResetSystime MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@end

MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRElectricalEnergyMeasurementClusterEnergyMeasurementStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull energy MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nullable startTimestamp MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nullable endTimestamp MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nullable startSystime MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) NSNumber * _Nullable endSystime MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@end

MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRElectricalEnergyMeasurementClusterCumulativeEnergyMeasuredEvent : NSObject <NSCopying>
@property (nonatomic, copy) MTRElectricalEnergyMeasurementClusterEnergyMeasurementStruct * _Nullable energyImported MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) MTRElectricalEnergyMeasurementClusterEnergyMeasurementStruct * _Nullable energyExported MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@end

MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRElectricalEnergyMeasurementClusterPeriodicEnergyMeasuredEvent : NSObject <NSCopying>
@property (nonatomic, copy) MTRElectricalEnergyMeasurementClusterEnergyMeasurementStruct * _Nullable energyImported MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@property (nonatomic, copy) MTRElectricalEnergyMeasurementClusterEnergyMeasurementStruct * _Nullable energyExported MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRWaterHeaterManagementClusterWaterHeaterBoostInfoStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull duration MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable oneShot MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable emergencyBoost MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable temporarySetpoint MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable targetPercentage MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable targetReheat MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRWaterHeaterManagementClusterBoostStartedEvent : NSObject <NSCopying>
@property (nonatomic, copy) MTRWaterHeaterManagementClusterWaterHeaterBoostInfoStruct * _Nonnull boostInfo MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRWaterHeaterManagementClusterBoostEndedEvent : NSObject <NSCopying>
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

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRMessagesClusterMessageResponseOptionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable messageResponseID MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSString * _Nullable label MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRMessagesClusterMessageStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSData * _Nonnull messageID MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull priority MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull messageControl MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable startTime MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable duration MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSString * _Nonnull messageText MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSArray * _Nullable responses MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRMessagesClusterMessageQueuedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSData * _Nonnull messageID MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRMessagesClusterMessagePresentedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSData * _Nonnull messageID MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRMessagesClusterMessageCompleteEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSData * _Nonnull messageID MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable responseID MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSString * _Nullable reply MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable futureMessagesPreference MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRDeviceEnergyManagementClusterCostStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull costType MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull value MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull decimalPoints MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable currency MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRDeviceEnergyManagementClusterPowerAdjustStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull minPower MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull maxPower MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull minDuration MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull maxDuration MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRDeviceEnergyManagementClusterPowerAdjustCapabilityStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nullable powerAdjustCapability MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull cause MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRDeviceEnergyManagementClusterSlotStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull minDuration MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull maxDuration MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull defaultDuration MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull elapsedSlotTime MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull remainingSlotTime MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable slotIsPausable MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable minPauseDuration MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable maxPauseDuration MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable manufacturerESAState MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable nominalPower MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable minPower MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable maxPower MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable nominalEnergy MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSArray * _Nullable costs MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable minPowerAdjustment MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable maxPowerAdjustment MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable minDurationAdjustment MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable maxDurationAdjustment MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRDeviceEnergyManagementClusterForecastStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull forecastID MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable activeSlotNumber MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull startTime MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull endTime MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable earliestStartTime MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable latestEndTime MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull isPausable MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSArray * _Nonnull slots MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull forecastUpdateReason MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRDeviceEnergyManagementClusterConstraintsStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull startTime MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull duration MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable nominalPower MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable maximumEnergy MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable loadControl MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRDeviceEnergyManagementClusterSlotAdjustmentStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull slotIndex MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable nominalPower MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull duration MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRDeviceEnergyManagementClusterPowerAdjustStartEvent : NSObject <NSCopying>
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRDeviceEnergyManagementClusterPowerAdjustEndEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull cause MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull duration MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull energyUse MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRDeviceEnergyManagementClusterPausedEvent : NSObject <NSCopying>
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRDeviceEnergyManagementClusterResumedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull cause MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTREnergyEVSEClusterChargingTargetStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull targetTimeMinutesPastMidnight MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable targetSoC MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable addedEnergy MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTREnergyEVSEClusterChargingTargetScheduleStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull dayOfWeekForSequence MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSArray * _Nonnull chargingTargets MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTREnergyEVSEClusterEVConnectedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull sessionID MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTREnergyEVSEClusterEVNotDetectedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull sessionID MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull state MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull sessionDuration MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull sessionEnergyCharged MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable sessionEnergyDischarged MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTREnergyEVSEClusterEnergyTransferStartedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull sessionID MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull state MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull maximumCurrent MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable maximumDischargeCurrent MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTREnergyEVSEClusterEnergyTransferStoppedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull sessionID MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull state MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull reason MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull energyTransferred MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable energyDischarged MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTREnergyEVSEClusterFaultEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable sessionID MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull state MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull faultStatePreviousState MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull faultStateCurrentState MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTREnergyEVSEClusterRFIDEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSData * _Nonnull uid MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTREnergyPreferenceClusterBalanceStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull step MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable label MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTREnergyEVSEModeClusterModeTagStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable mfgCode MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull value MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTREnergyEVSEModeClusterModeOptionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull label MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull mode MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSArray * _Nonnull modeTags MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRWaterHeaterModeClusterModeTagStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable mfgCode MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull value MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRWaterHeaterModeClusterModeOptionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull label MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull mode MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSArray * _Nonnull modeTags MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRDeviceEnergyManagementModeClusterModeTagStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable mfgCode MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull value MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRDeviceEnergyManagementModeClusterModeOptionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull label MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull mode MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSArray * _Nonnull modeTags MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
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

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRServiceAreaClusterLandmarkInfoStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull landmarkTag MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable relativePositionTag MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRServiceAreaClusterAreaInfoStruct : NSObject <NSCopying>
@property (nonatomic, copy) MTRDataTypeLocationDescriptorStruct * _Nullable locationInfo MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) MTRServiceAreaClusterLandmarkInfoStruct * _Nullable landmarkInfo MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRServiceAreaClusterAreaStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull areaID MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable mapID MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) MTRServiceAreaClusterAreaInfoStruct * _Nonnull areaInfo MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRServiceAreaClusterMapStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull mapID MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSString * _Nonnull name MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRServiceAreaClusterProgressStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull areaID MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull status MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable totalOperationalTime MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable estimatedTime MTR_PROVISIONALLY_AVAILABLE;
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

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRThermostatClusterScheduleTransitionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull dayOfWeek MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull transitionTime MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSData * _Nullable presetHandle MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable systemMode MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable coolingSetpoint MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable heatingSetpoint MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRThermostatClusterScheduleStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSData * _Nullable scheduleHandle MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull systemMode MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSString * _Nullable name MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSData * _Nullable presetHandle MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSArray * _Nonnull transitions MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable builtIn MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRThermostatClusterPresetStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSData * _Nullable presetHandle MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull presetScenario MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSString * _Nullable name MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable coolingSetpoint MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable heatingSetpoint MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable builtIn MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRThermostatClusterPresetTypeStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull presetScenario MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull numberOfPresets MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull presetTypeFeatures MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRThermostatClusterScheduleTypeStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull systemMode MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull numberOfSchedules MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull scheduleTypeFeatures MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
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

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTROccupancySensingClusterHoldTimeLimitsStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull holdTimeMin MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull holdTimeMax MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull holdTimeDefault MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTROccupancySensingClusterOccupancyChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull occupancy MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRThreadNetworkDirectoryClusterThreadNetworkStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSData * _Nonnull extendedPanID MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSString * _Nonnull networkName MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull channel MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull activeTimestamp MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRChannelClusterProgramCastStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull name MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSString * _Nonnull role MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRChannelClusterProgramCategoryStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull category MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSString * _Nullable subCategory MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRChannelClusterSeriesInfoStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull season MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSString * _Nonnull episode MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRChannelClusterChannelInfoStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull majorNumber MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * _Nonnull minorNumber MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nullable name MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nullable callSign MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nullable affiliateCallSign MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSString * _Nullable identifier MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable type MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_DEPRECATED("Please use MTRChannelClusterChannelInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRChannelClusterChannelInfo : MTRChannelClusterChannelInfoStruct
@property (nonatomic, copy) NSNumber * _Nonnull majorNumber MTR_DEPRECATED("Please use MTRChannelClusterChannelInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * _Nonnull minorNumber MTR_DEPRECATED("Please use MTRChannelClusterChannelInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSString * _Nullable name MTR_DEPRECATED("Please use MTRChannelClusterChannelInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSString * _Nullable callSign MTR_DEPRECATED("Please use MTRChannelClusterChannelInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSString * _Nullable affiliateCallSign MTR_DEPRECATED("Please use MTRChannelClusterChannelInfoStruct", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRChannelClusterProgramStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull identifier MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) MTRChannelClusterChannelInfoStruct * _Nonnull channel MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull startTime MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull endTime MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSString * _Nonnull title MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSString * _Nullable subtitle MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSString * _Nullable descriptionString MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSArray * _Nullable audioLanguages MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSArray * _Nullable ratings MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSString * _Nullable thumbnailUrl MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable posterArtUrl MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable dvbiUrl MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable releaseDate MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSString * _Nullable parentalGuidanceText MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nullable recordingFlag MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) MTRChannelClusterSeriesInfoStruct * _Nullable seriesInfo MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSArray * _Nullable categoryList MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSArray * _Nullable castList MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSArray * _Nullable externalIDList MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRChannelClusterPageTokenStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable limit MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSString * _Nullable after MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSString * _Nullable before MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRChannelClusterChannelPagingStruct : NSObject <NSCopying>
@property (nonatomic, copy) MTRChannelClusterPageTokenStruct * _Nullable previousToken MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) MTRChannelClusterPageTokenStruct * _Nullable nextToken MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
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

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRTargetNavigatorClusterTargetUpdatedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull targetList MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull currentTarget MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSData * _Nonnull data MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
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

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRMediaPlaybackClusterStateChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull currentState MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull startTime MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull duration MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) MTRMediaPlaybackClusterPlaybackPositionStruct * _Nonnull sampledPosition MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull playbackSpeed MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull seekRangeEnd MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull seekRangeStart MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSData * _Nullable data MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull audioAdvanceUnmuted MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
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

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRAccountLoginClusterLoggedOutEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable node MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRContentControlClusterRatingNameStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull ratingName MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable ratingNameDesc MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRContentControlClusterRemainingScreenTimeExpiredEvent : NSObject <NSCopying>
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRZoneManagementClusterTwoDCartesianVertexStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull x MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull y MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRZoneManagementClusterTwoDCartesianZoneStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull name MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull use MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nonnull vertices MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable color MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRZoneManagementClusterZoneInformationStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull zoneID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull zoneType MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull zoneSource MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRZoneManagementClusterZoneTriggerControlStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull initialDuration MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull augmentationDuration MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull maxDuration MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull blindDuration MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable sensitivity MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRZoneManagementClusterZoneTriggeredEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull zones MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull reason MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRZoneManagementClusterZoneStoppedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull zones MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull reason MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRCameraAVStreamManagementClusterVideoResolutionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull width MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull height MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRCameraAVStreamManagementClusterVideoStreamStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull videoStreamID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull streamUsage MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull videoCodec MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull minFrameRate MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull maxFrameRate MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRCameraAVStreamManagementClusterVideoResolutionStruct * _Nonnull minResolution MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRCameraAVStreamManagementClusterVideoResolutionStruct * _Nonnull maxResolution MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull minBitRate MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull maxBitRate MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull minFragmentLen MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull maxFragmentLen MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable watermarkEnabled MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable osdEnabled MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull referenceCount MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRCameraAVStreamManagementClusterSnapshotStreamStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull snapshotStreamID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull imageCodec MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull frameRate MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull bitRate MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRCameraAVStreamManagementClusterVideoResolutionStruct * _Nonnull minResolution MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRCameraAVStreamManagementClusterVideoResolutionStruct * _Nonnull maxResolution MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull quality MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull referenceCount MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRCameraAVStreamManagementClusterSnapshotParamsStruct : NSObject <NSCopying>
@property (nonatomic, copy) MTRCameraAVStreamManagementClusterVideoResolutionStruct * _Nonnull resolution MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull maxFrameRate MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull imageCodec MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRCameraAVStreamManagementClusterRateDistortionTradeOffPointsStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull codec MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRCameraAVStreamManagementClusterVideoResolutionStruct * _Nonnull resolution MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull minBitRate MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRCameraAVStreamManagementClusterAudioCapabilitiesStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull maxNumberOfChannels MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nonnull supportedCodecs MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nonnull supportedSampleRates MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nonnull supportedBitDepths MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRCameraAVStreamManagementClusterAudioStreamStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull audioStreamID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull streamUsage MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull audioCodec MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull channelCount MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull sampleRate MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull bitRate MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull bitDepth MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull referenceCount MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRCameraAVStreamManagementClusterVideoSensorParamsStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull sensorWidth MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull sensorHeight MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull hdrCapable MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull maxFPS MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull maxHDRFPS MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRCameraAVStreamManagementClusterViewportStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull x1 MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull y1 MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull x2 MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull y2 MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRCameraAVStreamManagementClusterVideoStreamChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull videoStreamID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable streamUsage MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable videoCodec MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable minFrameRate MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable maxFrameRate MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRCameraAVStreamManagementClusterVideoResolutionStruct * _Nullable minResolution MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRCameraAVStreamManagementClusterVideoResolutionStruct * _Nullable maxResolution MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable minBitRate MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable maxBitRate MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable minFragmentLen MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable maxFragmentLen MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRCameraAVStreamManagementClusterAudioStreamChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull audioStreamID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable streamUsage MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable audioCodec MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable channelCount MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable sampleRate MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable bitRate MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable bitDepth MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRCameraAVStreamManagementClusterSnapshotStreamChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull snapshotStreamID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable imageCodec MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable frameRate MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable bitRate MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRCameraAVStreamManagementClusterVideoResolutionStruct * _Nullable minResolution MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRCameraAVStreamManagementClusterVideoResolutionStruct * _Nullable maxResolution MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable quality MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRCameraAVSettingsUserLevelManagementClusterMPTZStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable pan MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable tilt MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable zoom MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRCameraAVSettingsUserLevelManagementClusterMPTZPresetStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull presetID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nonnull name MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRCameraAVSettingsUserLevelManagementClusterMPTZStruct * _Nonnull settings MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRCameraAVSettingsUserLevelManagementClusterViewportStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull x1 MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull y1 MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull x2 MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull y2 MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRWebRTCTransportProviderClusterICEServerStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull urls MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable username MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable credential MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable caid MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRWebRTCTransportProviderClusterWebRTCSessionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull id MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull peerNodeID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull peerFabricIndex MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull streamUsage MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable videoStreamID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable audioStreamID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull metadataOptions MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRWebRTCTransportRequestorClusterICEServerStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull urls MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable username MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nullable credential MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable caid MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRWebRTCTransportRequestorClusterWebRTCSessionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull id MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull peerNodeID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull peerFabricIndex MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull streamUsage MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable videoStreamID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable audioStreamID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull metadataOptions MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRPushAVStreamTransportClusterTransportMotionTriggerTimeControlStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull initialDuration MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull augmentationDuration MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull maxDuration MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull blindDuration MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRPushAVStreamTransportClusterTransportZoneOptionsStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable zone MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable sensitivity MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRPushAVStreamTransportClusterMetadataOptionsStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull multiplexing MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull includeMotionZones MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull enableMetadataPrivacySensitive MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRPushAVStreamTransportClusterTransportTriggerOptionsStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull triggerType MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nullable motionZones MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable motionSensitivity MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRPushAVStreamTransportClusterTransportMotionTriggerTimeControlStruct * _Nullable motionTimeControl MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable maxPreRollLen MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRPushAVStreamTransportClusterCMAFContainerOptionsStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull chunkDuration MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSData * _Nullable cencKey MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRPushAVStreamTransportClusterContainerOptionsStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull containerType MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRPushAVStreamTransportClusterCMAFContainerOptionsStruct * _Nullable cmafContainerOptions MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRPushAVStreamTransportClusterTransportOptionsStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull streamUsage MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable videoStreamID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable audioStreamID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull endpointID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nonnull url MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRPushAVStreamTransportClusterTransportTriggerOptionsStruct * _Nonnull triggerOptions MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull ingestMethod MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull containerFormat MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRPushAVStreamTransportClusterContainerOptionsStruct * _Nonnull containerOptions MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRPushAVStreamTransportClusterMetadataOptionsStruct * _Nullable metadataOptions MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable expiryTime MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRPushAVStreamTransportClusterTransportConfigurationStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull connectionID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull transportStatus MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRPushAVStreamTransportClusterTransportOptionsStruct * _Nonnull transportOptions MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRPushAVStreamTransportClusterPushTransportBeginEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull connectionID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull triggerType MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable activationReason MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRPushAVStreamTransportClusterPushTransportEndEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull connectionID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull triggerType MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable activationReason MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRChimeClusterChimeSoundStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull chimeID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSString * _Nonnull name MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTREcosystemInformationClusterDeviceTypeStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull deviceType MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull revision MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTREcosystemInformationClusterEcosystemDeviceStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nullable deviceName MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nullable deviceNameLastEdit MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull bridgedEndpoint MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull originalEndpoint MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nonnull deviceTypes MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nonnull uniqueLocationIDs MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull uniqueLocationIDsLastEdit MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTREcosystemInformationClusterEcosystemLocationStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull uniqueLocationID MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) MTRDataTypeLocationDescriptorStruct * _Nonnull locationDescriptor MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull locationDescriptorLastEdit MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRCommissionerControlClusterCommissioningRequestResultEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull requestID MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull clientNodeID MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull statusCode MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRTLSCertificateManagementClusterTLSCertStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull caid MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSData * _Nonnull certificate MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRTLSCertificateManagementClusterTLSClientCertificateDetailStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull ccdid MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSData * _Nonnull clientCertificate MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSArray * _Nonnull intermediateCertificates MTR_PROVISIONALLY_AVAILABLE;
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
@property (nonatomic, copy) NSNumber * _Nullable i MTR_PROVISIONALLY_AVAILABLE;
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
@property (nonatomic, copy) MTRDataTypeTestGlobalStruct * _Nullable d MTR_PROVISIONALLY_AVAILABLE;
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
@interface MTRUnitTestingClusterTestDifferentVendorMeiEventEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull arg1 MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRSampleMEIClusterPingCountEventEvent : NSObject <NSCopying>
@property (nonatomic, copy, getter=getCount) NSNumber * _Nonnull count MTR_PROVISIONALLY_AVAILABLE;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_PROVISIONALLY_AVAILABLE;
@end

NS_ASSUME_NONNULL_END
