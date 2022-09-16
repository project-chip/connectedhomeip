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

@interface MTRScenesClusterAttributeValuePair : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable attributeId;
@property (nonatomic, copy) NSArray * _Nonnull attributeValue;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRScenesClusterExtensionFieldSet : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull clusterId;
@property (nonatomic, copy) NSArray * _Nonnull attributeValueList;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRDescriptorClusterDeviceTypeStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull type;
@property (nonatomic, copy) NSNumber * _Nonnull revision;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRBindingClusterTargetStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable node;
@property (nonatomic, copy) NSNumber * _Nullable group;
@property (nonatomic, copy) NSNumber * _Nullable endpoint;
@property (nonatomic, copy) NSNumber * _Nullable cluster;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRAccessControlClusterTarget : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable cluster;
@property (nonatomic, copy) NSNumber * _Nullable endpoint;
@property (nonatomic, copy) NSNumber * _Nullable deviceType;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRAccessControlClusterAccessControlEntry : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull privilege;
@property (nonatomic, copy) NSNumber * _Nonnull authMode;
@property (nonatomic, copy) NSArray * _Nullable subjects;
@property (nonatomic, copy) NSArray * _Nullable targets;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRAccessControlClusterExtensionEntry : NSObject <NSCopying>
@property (nonatomic, copy) NSData * _Nonnull data;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRAccessControlClusterAccessControlEntryChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable adminNodeID;
@property (nonatomic, copy) NSNumber * _Nullable adminPasscodeID;
@property (nonatomic, copy) NSNumber * _Nonnull changeType;
@property (nonatomic, copy) MTRAccessControlClusterAccessControlEntry * _Nullable latestValue;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRAccessControlClusterAccessControlExtensionChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable adminNodeID;
@property (nonatomic, copy) NSNumber * _Nullable adminPasscodeID;
@property (nonatomic, copy) NSNumber * _Nonnull changeType;
@property (nonatomic, copy) MTRAccessControlClusterExtensionEntry * _Nullable latestValue;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRActionsClusterActionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull actionID;
@property (nonatomic, copy) NSString * _Nonnull name;
@property (nonatomic, copy) NSNumber * _Nonnull type;
@property (nonatomic, copy) NSNumber * _Nonnull endpointListID;
@property (nonatomic, copy) NSNumber * _Nonnull supportedCommands;
@property (nonatomic, copy) NSNumber * _Nonnull state;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRActionsClusterEndpointListStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull endpointListID;
@property (nonatomic, copy) NSString * _Nonnull name;
@property (nonatomic, copy) NSNumber * _Nonnull type;
@property (nonatomic, copy) NSArray * _Nonnull endpoints;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRActionsClusterStateChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull actionID;
@property (nonatomic, copy) NSNumber * _Nonnull invokeID;
@property (nonatomic, copy, getter=getNewState) NSNumber * _Nonnull newState;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRActionsClusterActionFailedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull actionID;
@property (nonatomic, copy) NSNumber * _Nonnull invokeID;
@property (nonatomic, copy, getter=getNewState) NSNumber * _Nonnull newState;
@property (nonatomic, copy) NSNumber * _Nonnull error;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRBasicClusterCapabilityMinimaStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull caseSessionsPerFabric;
@property (nonatomic, copy) NSNumber * _Nonnull subscriptionsPerFabric;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRBasicClusterStartUpEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRBasicClusterShutDownEvent : NSObject <NSCopying>

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRBasicClusterLeaveEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRBasicClusterReachableChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull reachableNewValue;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTROTASoftwareUpdateRequestorClusterProviderLocation : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull providerNodeID;
@property (nonatomic, copy) NSNumber * _Nonnull endpoint;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTROTASoftwareUpdateRequestorClusterStateTransitionEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull previousState;
@property (nonatomic, copy, getter=getNewState) NSNumber * _Nonnull newState;
@property (nonatomic, copy) NSNumber * _Nonnull reason;
@property (nonatomic, copy) NSNumber * _Nullable targetSoftwareVersion;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTROTASoftwareUpdateRequestorClusterVersionAppliedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion;
@property (nonatomic, copy) NSNumber * _Nonnull productID;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTROTASoftwareUpdateRequestorClusterDownloadErrorEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion;
@property (nonatomic, copy) NSNumber * _Nonnull bytesDownloaded;
@property (nonatomic, copy) NSNumber * _Nullable progressPercent;
@property (nonatomic, copy) NSNumber * _Nullable platformCode;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRPowerSourceClusterBatChargeFaultChangeType : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current;
@property (nonatomic, copy) NSArray * _Nonnull previous;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRPowerSourceClusterBatFaultChangeType : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current;
@property (nonatomic, copy) NSArray * _Nonnull previous;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRPowerSourceClusterWiredFaultChangeType : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current;
@property (nonatomic, copy) NSArray * _Nonnull previous;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRGeneralCommissioningClusterBasicCommissioningInfo : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull failSafeExpiryLengthSeconds;
@property (nonatomic, copy) NSNumber * _Nonnull maxCumulativeFailsafeSeconds;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRNetworkCommissioningClusterNetworkInfo : NSObject <NSCopying>
@property (nonatomic, copy) NSData * _Nonnull networkID;
@property (nonatomic, copy) NSNumber * _Nonnull connected;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRNetworkCommissioningClusterThreadInterfaceScanResult : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull panId;
@property (nonatomic, copy) NSNumber * _Nonnull extendedPanId;
@property (nonatomic, copy) NSString * _Nonnull networkName;
@property (nonatomic, copy) NSNumber * _Nonnull channel;
@property (nonatomic, copy) NSNumber * _Nonnull version;
@property (nonatomic, copy) NSData * _Nonnull extendedAddress;
@property (nonatomic, copy) NSNumber * _Nonnull rssi;
@property (nonatomic, copy) NSNumber * _Nonnull lqi;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRNetworkCommissioningClusterWiFiInterfaceScanResult : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull security;
@property (nonatomic, copy) NSData * _Nonnull ssid;
@property (nonatomic, copy) NSData * _Nonnull bssid;
@property (nonatomic, copy) NSNumber * _Nonnull channel;
@property (nonatomic, copy) NSNumber * _Nonnull wiFiBand;
@property (nonatomic, copy) NSNumber * _Nonnull rssi;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRGeneralDiagnosticsClusterNetworkInterfaceType : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull name;
@property (nonatomic, copy) NSNumber * _Nonnull isOperational;
@property (nonatomic, copy) NSNumber * _Nullable offPremiseServicesReachableIPv4;
@property (nonatomic, copy) NSNumber * _Nullable offPremiseServicesReachableIPv6;
@property (nonatomic, copy) NSData * _Nonnull hardwareAddress;
@property (nonatomic, copy) NSArray * _Nonnull iPv4Addresses;
@property (nonatomic, copy) NSArray * _Nonnull iPv6Addresses;
@property (nonatomic, copy) NSNumber * _Nonnull type;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRGeneralDiagnosticsClusterHardwareFaultChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current;
@property (nonatomic, copy) NSArray * _Nonnull previous;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRGeneralDiagnosticsClusterRadioFaultChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current;
@property (nonatomic, copy) NSArray * _Nonnull previous;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRGeneralDiagnosticsClusterNetworkFaultChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current;
@property (nonatomic, copy) NSArray * _Nonnull previous;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRGeneralDiagnosticsClusterBootReasonEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull bootReason;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRSoftwareDiagnosticsClusterThreadMetrics : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull id;
@property (nonatomic, copy) NSString * _Nullable name;
@property (nonatomic, copy) NSNumber * _Nullable stackFreeCurrent;
@property (nonatomic, copy) NSNumber * _Nullable stackFreeMinimum;
@property (nonatomic, copy) NSNumber * _Nullable stackSize;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRSoftwareDiagnosticsClusterSoftwareFaultEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull id;
@property (nonatomic, copy) NSString * _Nullable name;
@property (nonatomic, copy) NSData * _Nullable faultRecording;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRThreadNetworkDiagnosticsClusterNeighborTable : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull extAddress;
@property (nonatomic, copy) NSNumber * _Nonnull age;
@property (nonatomic, copy) NSNumber * _Nonnull rloc16;
@property (nonatomic, copy) NSNumber * _Nonnull linkFrameCounter;
@property (nonatomic, copy) NSNumber * _Nonnull mleFrameCounter;
@property (nonatomic, copy) NSNumber * _Nonnull lqi;
@property (nonatomic, copy) NSNumber * _Nullable averageRssi;
@property (nonatomic, copy) NSNumber * _Nullable lastRssi;
@property (nonatomic, copy) NSNumber * _Nonnull frameErrorRate;
@property (nonatomic, copy) NSNumber * _Nonnull messageErrorRate;
@property (nonatomic, copy) NSNumber * _Nonnull rxOnWhenIdle;
@property (nonatomic, copy) NSNumber * _Nonnull fullThreadDevice;
@property (nonatomic, copy) NSNumber * _Nonnull fullNetworkData;
@property (nonatomic, copy) NSNumber * _Nonnull isChild;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRThreadNetworkDiagnosticsClusterOperationalDatasetComponents : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull activeTimestampPresent;
@property (nonatomic, copy) NSNumber * _Nonnull pendingTimestampPresent;
@property (nonatomic, copy) NSNumber * _Nonnull masterKeyPresent;
@property (nonatomic, copy) NSNumber * _Nonnull networkNamePresent;
@property (nonatomic, copy) NSNumber * _Nonnull extendedPanIdPresent;
@property (nonatomic, copy) NSNumber * _Nonnull meshLocalPrefixPresent;
@property (nonatomic, copy) NSNumber * _Nonnull delayPresent;
@property (nonatomic, copy) NSNumber * _Nonnull panIdPresent;
@property (nonatomic, copy) NSNumber * _Nonnull channelPresent;
@property (nonatomic, copy) NSNumber * _Nonnull pskcPresent;
@property (nonatomic, copy) NSNumber * _Nonnull securityPolicyPresent;
@property (nonatomic, copy) NSNumber * _Nonnull channelMaskPresent;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRThreadNetworkDiagnosticsClusterRouteTable : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull extAddress;
@property (nonatomic, copy) NSNumber * _Nonnull rloc16;
@property (nonatomic, copy) NSNumber * _Nonnull routerId;
@property (nonatomic, copy) NSNumber * _Nonnull nextHop;
@property (nonatomic, copy) NSNumber * _Nonnull pathCost;
@property (nonatomic, copy) NSNumber * _Nonnull lqiIn;
@property (nonatomic, copy) NSNumber * _Nonnull lqiOut;
@property (nonatomic, copy) NSNumber * _Nonnull age;
@property (nonatomic, copy) NSNumber * _Nonnull allocated;
@property (nonatomic, copy) NSNumber * _Nonnull linkEstablished;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRThreadNetworkDiagnosticsClusterSecurityPolicy : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull rotationTime;
@property (nonatomic, copy) NSNumber * _Nonnull flags;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRThreadNetworkDiagnosticsClusterConnectionStatusEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull connectionStatus;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRThreadNetworkDiagnosticsClusterNetworkFaultChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current;
@property (nonatomic, copy) NSArray * _Nonnull previous;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRWiFiNetworkDiagnosticsClusterDisconnectionEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull reasonCode;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRWiFiNetworkDiagnosticsClusterAssociationFailureEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull associationFailure;
@property (nonatomic, copy) NSNumber * _Nonnull status;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRWiFiNetworkDiagnosticsClusterConnectionStatusEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull connectionStatus;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRTimeSynchronizationClusterDstOffsetType : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull offset;
@property (nonatomic, copy) NSNumber * _Nonnull validStarting;
@property (nonatomic, copy) NSNumber * _Nonnull validUntil;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRTimeSynchronizationClusterTimeZoneType : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull offset;
@property (nonatomic, copy) NSNumber * _Nonnull validAt;
@property (nonatomic, copy) NSString * _Nullable name;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRBridgedDeviceBasicClusterStartUpEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRBridgedDeviceBasicClusterShutDownEvent : NSObject <NSCopying>

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRBridgedDeviceBasicClusterLeaveEvent : NSObject <NSCopying>

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRBridgedDeviceBasicClusterReachableChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull reachableNewValue;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRSwitchClusterSwitchLatchedEvent : NSObject <NSCopying>
@property (nonatomic, copy, getter=getNewPosition) NSNumber * _Nonnull newPosition;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRSwitchClusterInitialPressEvent : NSObject <NSCopying>
@property (nonatomic, copy, getter=getNewPosition) NSNumber * _Nonnull newPosition;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRSwitchClusterLongPressEvent : NSObject <NSCopying>
@property (nonatomic, copy, getter=getNewPosition) NSNumber * _Nonnull newPosition;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRSwitchClusterShortReleaseEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull previousPosition;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRSwitchClusterLongReleaseEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull previousPosition;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRSwitchClusterMultiPressOngoingEvent : NSObject <NSCopying>
@property (nonatomic, copy, getter=getNewPosition) NSNumber * _Nonnull newPosition;
@property (nonatomic, copy) NSNumber * _Nonnull currentNumberOfPressesCounted;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRSwitchClusterMultiPressCompleteEvent : NSObject <NSCopying>
@property (nonatomic, copy, getter=getNewPosition) NSNumber * _Nonnull newPosition;
@property (nonatomic, copy) NSNumber * _Nonnull totalNumberOfPressesCounted;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTROperationalCredentialsClusterFabricDescriptor : NSObject <NSCopying>
@property (nonatomic, copy) NSData * _Nonnull rootPublicKey;
@property (nonatomic, copy) NSNumber * _Nonnull vendorId;
@property (nonatomic, copy) NSNumber * _Nonnull fabricId;
@property (nonatomic, copy) NSNumber * _Nonnull nodeId;
@property (nonatomic, copy) NSString * _Nonnull label;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTROperationalCredentialsClusterNOCStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSData * _Nonnull noc;
@property (nonatomic, copy) NSData * _Nullable icac;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRGroupKeyManagementClusterGroupInfoMapStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull groupId;
@property (nonatomic, copy) NSArray * _Nonnull endpoints;
@property (nonatomic, copy) NSString * _Nullable groupName;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRGroupKeyManagementClusterGroupKeyMapStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull groupId;
@property (nonatomic, copy) NSNumber * _Nonnull groupKeySetID;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRGroupKeyManagementClusterGroupKeySetStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull groupKeySetID;
@property (nonatomic, copy) NSNumber * _Nonnull groupKeySecurityPolicy;
@property (nonatomic, copy) NSData * _Nullable epochKey0;
@property (nonatomic, copy) NSNumber * _Nullable epochStartTime0;
@property (nonatomic, copy) NSData * _Nullable epochKey1;
@property (nonatomic, copy) NSNumber * _Nullable epochStartTime1;
@property (nonatomic, copy) NSData * _Nullable epochKey2;
@property (nonatomic, copy) NSNumber * _Nullable epochStartTime2;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRFixedLabelClusterLabelStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull label;
@property (nonatomic, copy) NSString * _Nonnull value;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRUserLabelClusterLabelStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull label;
@property (nonatomic, copy) NSString * _Nonnull value;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRBooleanStateClusterStateChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull stateValue;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRModeSelectClusterSemanticTag : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull mfgCode;
@property (nonatomic, copy) NSNumber * _Nonnull value;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRModeSelectClusterModeOptionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull label;
@property (nonatomic, copy) NSNumber * _Nonnull mode;
@property (nonatomic, copy) NSArray * _Nonnull semanticTags;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRDoorLockClusterDlCredential : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull credentialType;
@property (nonatomic, copy) NSNumber * _Nonnull credentialIndex;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRDoorLockClusterDoorLockAlarmEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull alarmCode;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRDoorLockClusterDoorStateChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull doorState;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRDoorLockClusterLockOperationEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull lockOperationType;
@property (nonatomic, copy) NSNumber * _Nonnull operationSource;
@property (nonatomic, copy) NSNumber * _Nullable userIndex;
@property (nonatomic, copy) NSNumber * _Nullable fabricIndex;
@property (nonatomic, copy) NSNumber * _Nullable sourceNode;
@property (nonatomic, copy) NSArray * _Nullable credentials;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRDoorLockClusterLockOperationErrorEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull lockOperationType;
@property (nonatomic, copy) NSNumber * _Nonnull operationSource;
@property (nonatomic, copy) NSNumber * _Nonnull operationError;
@property (nonatomic, copy) NSNumber * _Nullable userIndex;
@property (nonatomic, copy) NSNumber * _Nullable fabricIndex;
@property (nonatomic, copy) NSNumber * _Nullable sourceNode;
@property (nonatomic, copy) NSArray * _Nullable credentials;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRDoorLockClusterLockUserChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull lockDataType;
@property (nonatomic, copy) NSNumber * _Nonnull dataOperationType;
@property (nonatomic, copy) NSNumber * _Nonnull operationSource;
@property (nonatomic, copy) NSNumber * _Nullable userIndex;
@property (nonatomic, copy) NSNumber * _Nullable fabricIndex;
@property (nonatomic, copy) NSNumber * _Nullable sourceNode;
@property (nonatomic, copy) NSNumber * _Nullable dataIndex;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRPumpConfigurationAndControlClusterSupplyVoltageLowEvent : NSObject <NSCopying>

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRPumpConfigurationAndControlClusterSupplyVoltageHighEvent : NSObject <NSCopying>

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRPumpConfigurationAndControlClusterPowerMissingPhaseEvent : NSObject <NSCopying>

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRPumpConfigurationAndControlClusterSystemPressureLowEvent : NSObject <NSCopying>

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRPumpConfigurationAndControlClusterSystemPressureHighEvent : NSObject <NSCopying>

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRPumpConfigurationAndControlClusterDryRunningEvent : NSObject <NSCopying>

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRPumpConfigurationAndControlClusterMotorTemperatureHighEvent : NSObject <NSCopying>

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRPumpConfigurationAndControlClusterPumpMotorFatalFailureEvent : NSObject <NSCopying>

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRPumpConfigurationAndControlClusterElectronicTemperatureHighEvent : NSObject <NSCopying>

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRPumpConfigurationAndControlClusterPumpBlockedEvent : NSObject <NSCopying>

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRPumpConfigurationAndControlClusterSensorFailureEvent : NSObject <NSCopying>

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRPumpConfigurationAndControlClusterElectronicNonFatalFailureEvent : NSObject <NSCopying>

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRPumpConfigurationAndControlClusterElectronicFatalFailureEvent : NSObject <NSCopying>

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRPumpConfigurationAndControlClusterGeneralFaultEvent : NSObject <NSCopying>

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRPumpConfigurationAndControlClusterLeakageEvent : NSObject <NSCopying>

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRPumpConfigurationAndControlClusterAirDetectionEvent : NSObject <NSCopying>

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRPumpConfigurationAndControlClusterTurbineOperationEvent : NSObject <NSCopying>

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRThermostatClusterThermostatScheduleTransition : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull transitionTime;
@property (nonatomic, copy) NSNumber * _Nullable heatSetpoint;
@property (nonatomic, copy) NSNumber * _Nullable coolSetpoint;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRChannelClusterChannelInfo : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull majorNumber;
@property (nonatomic, copy) NSNumber * _Nonnull minorNumber;
@property (nonatomic, copy) NSString * _Nullable name;
@property (nonatomic, copy) NSString * _Nullable callSign;
@property (nonatomic, copy) NSString * _Nullable affiliateCallSign;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRChannelClusterLineupInfo : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull operatorName;
@property (nonatomic, copy) NSString * _Nullable lineupName;
@property (nonatomic, copy) NSString * _Nullable postalCode;
@property (nonatomic, copy) NSNumber * _Nonnull lineupInfoType;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRTargetNavigatorClusterTargetInfo : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull identifier;
@property (nonatomic, copy) NSString * _Nonnull name;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRMediaPlaybackClusterPlaybackPosition : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull updatedAt;
@property (nonatomic, copy) NSNumber * _Nullable position;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRMediaInputClusterInputInfo : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull index;
@property (nonatomic, copy) NSNumber * _Nonnull inputType;
@property (nonatomic, copy) NSString * _Nonnull name;
@property (nonatomic, copy) NSString * _Nonnull descriptionString;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRContentLauncherClusterDimension : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull width;
@property (nonatomic, copy) NSNumber * _Nonnull height;
@property (nonatomic, copy) NSNumber * _Nonnull metric;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRContentLauncherClusterAdditionalInfo : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull name;
@property (nonatomic, copy) NSString * _Nonnull value;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRContentLauncherClusterParameter : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull type;
@property (nonatomic, copy) NSString * _Nonnull value;
@property (nonatomic, copy) NSArray * _Nullable externalIDList;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRContentLauncherClusterContentSearch : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull parameterList;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRContentLauncherClusterStyleInformation : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nullable imageUrl;
@property (nonatomic, copy) NSString * _Nullable color;
@property (nonatomic, copy) MTRContentLauncherClusterDimension * _Nullable size;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRContentLauncherClusterBrandingInformation : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull providerName;
@property (nonatomic, copy) MTRContentLauncherClusterStyleInformation * _Nullable background;
@property (nonatomic, copy) MTRContentLauncherClusterStyleInformation * _Nullable logo;
@property (nonatomic, copy) MTRContentLauncherClusterStyleInformation * _Nullable progressBar;
@property (nonatomic, copy) MTRContentLauncherClusterStyleInformation * _Nullable splash;
@property (nonatomic, copy) MTRContentLauncherClusterStyleInformation * _Nullable waterMark;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRAudioOutputClusterOutputInfo : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull index;
@property (nonatomic, copy) NSNumber * _Nonnull outputType;
@property (nonatomic, copy) NSString * _Nonnull name;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRApplicationLauncherClusterApplication : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull catalogVendorId;
@property (nonatomic, copy) NSString * _Nonnull applicationId;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRApplicationLauncherClusterApplicationEP : NSObject <NSCopying>
@property (nonatomic, copy) MTRApplicationLauncherClusterApplication * _Nonnull application;
@property (nonatomic, copy) NSNumber * _Nullable endpoint;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRApplicationBasicClusterApplicationBasicApplication : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull catalogVendorId;
@property (nonatomic, copy) NSString * _Nonnull applicationId;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRTestClusterClusterSimpleStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull a;
@property (nonatomic, copy) NSNumber * _Nonnull b;
@property (nonatomic, copy) NSNumber * _Nonnull c;
@property (nonatomic, copy) NSData * _Nonnull d;
@property (nonatomic, copy) NSString * _Nonnull e;
@property (nonatomic, copy) NSNumber * _Nonnull f;
@property (nonatomic, copy) NSNumber * _Nonnull g;
@property (nonatomic, copy) NSNumber * _Nonnull h;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRTestClusterClusterTestFabricScoped : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull fabricSensitiveInt8u;
@property (nonatomic, copy) NSNumber * _Nullable optionalFabricSensitiveInt8u;
@property (nonatomic, copy) NSNumber * _Nullable nullableFabricSensitiveInt8u;
@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalFabricSensitiveInt8u;
@property (nonatomic, copy) NSString * _Nonnull fabricSensitiveCharString;
@property (nonatomic, copy) MTRTestClusterClusterSimpleStruct * _Nonnull fabricSensitiveStruct;
@property (nonatomic, copy) NSArray * _Nonnull fabricSensitiveInt8uList;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRTestClusterClusterNullablesAndOptionalsStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable nullableInt;
@property (nonatomic, copy) NSNumber * _Nullable optionalInt;
@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalInt;
@property (nonatomic, copy) NSString * _Nullable nullableString;
@property (nonatomic, copy) NSString * _Nullable optionalString;
@property (nonatomic, copy) NSString * _Nullable nullableOptionalString;
@property (nonatomic, copy) MTRTestClusterClusterSimpleStruct * _Nullable nullableStruct;
@property (nonatomic, copy) MTRTestClusterClusterSimpleStruct * _Nullable optionalStruct;
@property (nonatomic, copy) MTRTestClusterClusterSimpleStruct * _Nullable nullableOptionalStruct;
@property (nonatomic, copy) NSArray * _Nullable nullableList;
@property (nonatomic, copy) NSArray * _Nullable optionalList;
@property (nonatomic, copy) NSArray * _Nullable nullableOptionalList;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRTestClusterClusterNestedStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull a;
@property (nonatomic, copy) NSNumber * _Nonnull b;
@property (nonatomic, copy) MTRTestClusterClusterSimpleStruct * _Nonnull c;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRTestClusterClusterNestedStructList : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull a;
@property (nonatomic, copy) NSNumber * _Nonnull b;
@property (nonatomic, copy) MTRTestClusterClusterSimpleStruct * _Nonnull c;
@property (nonatomic, copy) NSArray * _Nonnull d;
@property (nonatomic, copy) NSArray * _Nonnull e;
@property (nonatomic, copy) NSArray * _Nonnull f;
@property (nonatomic, copy) NSArray * _Nonnull g;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRTestClusterClusterDoubleNestedStructList : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull a;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRTestClusterClusterTestListStructOctet : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull member1;
@property (nonatomic, copy) NSData * _Nonnull member2;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRTestClusterClusterTestEventEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull arg1;
@property (nonatomic, copy) NSNumber * _Nonnull arg2;
@property (nonatomic, copy) NSNumber * _Nonnull arg3;
@property (nonatomic, copy) MTRTestClusterClusterSimpleStruct * _Nonnull arg4;
@property (nonatomic, copy) NSArray * _Nonnull arg5;
@property (nonatomic, copy) NSArray * _Nonnull arg6;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

@interface MTRTestClusterClusterTestFabricScopedEventEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

NS_ASSUME_NONNULL_END
