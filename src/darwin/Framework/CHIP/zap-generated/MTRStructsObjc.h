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
@end

@interface MTRScenesClusterExtensionFieldSet : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull clusterId;
@property (nonatomic, copy) NSArray * _Nonnull attributeValueList;
@end

API_AVAILABLE(ios(16.2), macos(13.1), watchos(9.2), tvos(16.2))
@interface MTRDescriptorClusterDeviceTypeStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull type;
@property (nonatomic, copy) NSNumber * _Nonnull revision;
@end

@interface MTRDescriptorClusterDeviceType : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull type;
@property (nonatomic, copy) NSNumber * _Nonnull revision;
@end

@interface MTRBindingClusterTargetStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable node;
@property (nonatomic, copy) NSNumber * _Nullable group;
@property (nonatomic, copy) NSNumber * _Nullable endpoint;
@property (nonatomic, copy) NSNumber * _Nullable cluster;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex;
@end

@interface MTRAccessControlClusterTarget : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable cluster;
@property (nonatomic, copy) NSNumber * _Nullable endpoint;
@property (nonatomic, copy) NSNumber * _Nullable deviceType;
@end

@interface MTRAccessControlClusterAccessControlEntry : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull privilege;
@property (nonatomic, copy) NSNumber * _Nonnull authMode;
@property (nonatomic, copy) NSArray * _Nullable subjects;
@property (nonatomic, copy) NSArray * _Nullable targets;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex;
@end

@interface MTRAccessControlClusterExtensionEntry : NSObject <NSCopying>
@property (nonatomic, copy) NSData * _Nonnull data;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex;
@end

@interface MTRAccessControlClusterAccessControlEntryChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable adminNodeID;
@property (nonatomic, copy) NSNumber * _Nullable adminPasscodeID;
@property (nonatomic, copy) NSNumber * _Nonnull changeType;
@property (nonatomic, copy) MTRAccessControlClusterAccessControlEntry * _Nullable latestValue;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex;
@end

@interface MTRAccessControlClusterAccessControlExtensionChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable adminNodeID;
@property (nonatomic, copy) NSNumber * _Nullable adminPasscodeID;
@property (nonatomic, copy) NSNumber * _Nonnull changeType;
@property (nonatomic, copy) MTRAccessControlClusterExtensionEntry * _Nullable latestValue;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex;
@end

@interface MTRActionsClusterActionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull actionID;
@property (nonatomic, copy) NSString * _Nonnull name;
@property (nonatomic, copy) NSNumber * _Nonnull type;
@property (nonatomic, copy) NSNumber * _Nonnull endpointListID;
@property (nonatomic, copy) NSNumber * _Nonnull supportedCommands;
@property (nonatomic, copy) NSNumber * _Nonnull state;
@end

@interface MTRActionsClusterEndpointListStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull endpointListID;
@property (nonatomic, copy) NSString * _Nonnull name;
@property (nonatomic, copy) NSNumber * _Nonnull type;
@property (nonatomic, copy) NSArray * _Nonnull endpoints;
@end

@interface MTRActionsClusterStateChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull actionID;
@property (nonatomic, copy) NSNumber * _Nonnull invokeID;
@property (nonatomic, copy, getter=getNewState) NSNumber * _Nonnull newState;
@end

@interface MTRActionsClusterActionFailedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull actionID;
@property (nonatomic, copy) NSNumber * _Nonnull invokeID;
@property (nonatomic, copy, getter=getNewState) NSNumber * _Nonnull newState;
@property (nonatomic, copy) NSNumber * _Nonnull error;
@end

@interface MTRBasicClusterCapabilityMinimaStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull caseSessionsPerFabric;
@property (nonatomic, copy) NSNumber * _Nonnull subscriptionsPerFabric;
@end

@interface MTRBasicClusterStartUpEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion;
@end

@interface MTRBasicClusterShutDownEvent : NSObject <NSCopying>
@end

@interface MTRBasicClusterLeaveEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex;
@end

@interface MTRBasicClusterReachableChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull reachableNewValue;
@end

MTR_NEWLY_AVAILABLE
@interface MTROTASoftwareUpdateRequestorClusterProviderLocation : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull providerNodeID;
@property (nonatomic, copy) NSNumber * _Nonnull endpoint;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex;
@end

MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterProviderLocation")
@interface MTROtaSoftwareUpdateRequestorClusterProviderLocation : MTROTASoftwareUpdateRequestorClusterProviderLocation
@end

MTR_NEWLY_AVAILABLE
@interface MTROTASoftwareUpdateRequestorClusterStateTransitionEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull previousState;
@property (nonatomic, copy, getter=getNewState) NSNumber * _Nonnull newState;
@property (nonatomic, copy) NSNumber * _Nonnull reason;
@property (nonatomic, copy) NSNumber * _Nullable targetSoftwareVersion;
@end

MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterStateTransitionEvent")
@interface MTROtaSoftwareUpdateRequestorClusterStateTransitionEvent : MTROTASoftwareUpdateRequestorClusterStateTransitionEvent
@end

MTR_NEWLY_AVAILABLE
@interface MTROTASoftwareUpdateRequestorClusterVersionAppliedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion;
@property (nonatomic, copy) NSNumber * _Nonnull productID;
@end

MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterVersionAppliedEvent")
@interface MTROtaSoftwareUpdateRequestorClusterVersionAppliedEvent : MTROTASoftwareUpdateRequestorClusterVersionAppliedEvent
@end

MTR_NEWLY_AVAILABLE
@interface MTROTASoftwareUpdateRequestorClusterDownloadErrorEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion;
@property (nonatomic, copy) NSNumber * _Nonnull bytesDownloaded;
@property (nonatomic, copy) NSNumber * _Nullable progressPercent;
@property (nonatomic, copy) NSNumber * _Nullable platformCode;
@end

MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterDownloadErrorEvent")
@interface MTROtaSoftwareUpdateRequestorClusterDownloadErrorEvent : MTROTASoftwareUpdateRequestorClusterDownloadErrorEvent
@end

@interface MTRPowerSourceClusterBatChargeFaultChangeType : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current;
@property (nonatomic, copy) NSArray * _Nonnull previous;
@end

@interface MTRPowerSourceClusterBatFaultChangeType : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current;
@property (nonatomic, copy) NSArray * _Nonnull previous;
@end

@interface MTRPowerSourceClusterWiredFaultChangeType : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current;
@property (nonatomic, copy) NSArray * _Nonnull previous;
@end

@interface MTRGeneralCommissioningClusterBasicCommissioningInfo : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull failSafeExpiryLengthSeconds;
@property (nonatomic, copy) NSNumber * _Nonnull maxCumulativeFailsafeSeconds;
@end

@interface MTRNetworkCommissioningClusterNetworkInfo : NSObject <NSCopying>
@property (nonatomic, copy) NSData * _Nonnull networkID;
@property (nonatomic, copy) NSNumber * _Nonnull connected;
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
@end

@interface MTRNetworkCommissioningClusterWiFiInterfaceScanResult : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull security;
@property (nonatomic, copy) NSData * _Nonnull ssid;
@property (nonatomic, copy) NSData * _Nonnull bssid;
@property (nonatomic, copy) NSNumber * _Nonnull channel;
@property (nonatomic, copy) NSNumber * _Nonnull wiFiBand;
@property (nonatomic, copy) NSNumber * _Nonnull rssi;
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
@end

@interface MTRGeneralDiagnosticsClusterHardwareFaultChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current;
@property (nonatomic, copy) NSArray * _Nonnull previous;
@end

@interface MTRGeneralDiagnosticsClusterRadioFaultChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current;
@property (nonatomic, copy) NSArray * _Nonnull previous;
@end

@interface MTRGeneralDiagnosticsClusterNetworkFaultChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current;
@property (nonatomic, copy) NSArray * _Nonnull previous;
@end

@interface MTRGeneralDiagnosticsClusterBootReasonEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull bootReason;
@end

@interface MTRSoftwareDiagnosticsClusterThreadMetrics : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull id;
@property (nonatomic, copy) NSString * _Nullable name;
@property (nonatomic, copy) NSNumber * _Nullable stackFreeCurrent;
@property (nonatomic, copy) NSNumber * _Nullable stackFreeMinimum;
@property (nonatomic, copy) NSNumber * _Nullable stackSize;
@end

@interface MTRSoftwareDiagnosticsClusterSoftwareFaultEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull id;
@property (nonatomic, copy) NSString * _Nullable name;
@property (nonatomic, copy) NSData * _Nullable faultRecording;
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
@end

@interface MTRThreadNetworkDiagnosticsClusterSecurityPolicy : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull rotationTime;
@property (nonatomic, copy) NSNumber * _Nonnull flags;
@end

@interface MTRThreadNetworkDiagnosticsClusterConnectionStatusEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull connectionStatus;
@end

@interface MTRThreadNetworkDiagnosticsClusterNetworkFaultChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull current;
@property (nonatomic, copy) NSArray * _Nonnull previous;
@end

@interface MTRWiFiNetworkDiagnosticsClusterDisconnectionEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull reasonCode;
@end

@interface MTRWiFiNetworkDiagnosticsClusterAssociationFailureEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull associationFailure;
@property (nonatomic, copy) NSNumber * _Nonnull status;
@end

@interface MTRWiFiNetworkDiagnosticsClusterConnectionStatusEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull connectionStatus;
@end

@interface MTRTimeSynchronizationClusterDstOffsetType : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull offset;
@property (nonatomic, copy) NSNumber * _Nonnull validStarting;
@property (nonatomic, copy) NSNumber * _Nonnull validUntil;
@end

@interface MTRTimeSynchronizationClusterTimeZoneType : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull offset;
@property (nonatomic, copy) NSNumber * _Nonnull validAt;
@property (nonatomic, copy) NSString * _Nullable name;
@end

@interface MTRBridgedDeviceBasicClusterStartUpEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion;
@end

@interface MTRBridgedDeviceBasicClusterShutDownEvent : NSObject <NSCopying>
@end

@interface MTRBridgedDeviceBasicClusterLeaveEvent : NSObject <NSCopying>
@end

@interface MTRBridgedDeviceBasicClusterReachableChangedEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull reachableNewValue;
@end

@interface MTRSwitchClusterSwitchLatchedEvent : NSObject <NSCopying>
@property (nonatomic, copy, getter=getNewPosition) NSNumber * _Nonnull newPosition;
@end

@interface MTRSwitchClusterInitialPressEvent : NSObject <NSCopying>
@property (nonatomic, copy, getter=getNewPosition) NSNumber * _Nonnull newPosition;
@end

@interface MTRSwitchClusterLongPressEvent : NSObject <NSCopying>
@property (nonatomic, copy, getter=getNewPosition) NSNumber * _Nonnull newPosition;
@end

@interface MTRSwitchClusterShortReleaseEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull previousPosition;
@end

@interface MTRSwitchClusterLongReleaseEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull previousPosition;
@end

@interface MTRSwitchClusterMultiPressOngoingEvent : NSObject <NSCopying>
@property (nonatomic, copy, getter=getNewPosition) NSNumber * _Nonnull newPosition;
@property (nonatomic, copy) NSNumber * _Nonnull currentNumberOfPressesCounted;
@end

@interface MTRSwitchClusterMultiPressCompleteEvent : NSObject <NSCopying>
@property (nonatomic, copy, getter=getNewPosition) NSNumber * _Nonnull newPosition;
@property (nonatomic, copy) NSNumber * _Nonnull totalNumberOfPressesCounted;
@end

@interface MTROperationalCredentialsClusterFabricDescriptor : NSObject <NSCopying>
@property (nonatomic, copy) NSData * _Nonnull rootPublicKey;
@property (nonatomic, copy) NSNumber * _Nonnull vendorId;
@property (nonatomic, copy) NSNumber * _Nonnull fabricId;
@property (nonatomic, copy) NSNumber * _Nonnull nodeId;
@property (nonatomic, copy) NSString * _Nonnull label;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex;
@end

@interface MTROperationalCredentialsClusterNOCStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSData * _Nonnull noc;
@property (nonatomic, copy) NSData * _Nullable icac;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex;
@end

@interface MTRGroupKeyManagementClusterGroupInfoMapStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull groupId;
@property (nonatomic, copy) NSArray * _Nonnull endpoints;
@property (nonatomic, copy) NSString * _Nullable groupName;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex;
@end

@interface MTRGroupKeyManagementClusterGroupKeyMapStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull groupId;
@property (nonatomic, copy) NSNumber * _Nonnull groupKeySetID;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex;
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
@end

@interface MTRFixedLabelClusterLabelStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull label;
@property (nonatomic, copy) NSString * _Nonnull value;
@end

@interface MTRUserLabelClusterLabelStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull label;
@property (nonatomic, copy) NSString * _Nonnull value;
@end

@interface MTRBooleanStateClusterStateChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull stateValue;
@end

@interface MTRModeSelectClusterSemanticTag : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull mfgCode;
@property (nonatomic, copy) NSNumber * _Nonnull value;
@end

@interface MTRModeSelectClusterModeOptionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull label;
@property (nonatomic, copy) NSNumber * _Nonnull mode;
@property (nonatomic, copy) NSArray * _Nonnull semanticTags;
@end

@interface MTRDoorLockClusterDlCredential : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull credentialType;
@property (nonatomic, copy) NSNumber * _Nonnull credentialIndex;
@end

@interface MTRDoorLockClusterDoorLockAlarmEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull alarmCode;
@end

@interface MTRDoorLockClusterDoorStateChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull doorState;
@end

@interface MTRDoorLockClusterLockOperationEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull lockOperationType;
@property (nonatomic, copy) NSNumber * _Nonnull operationSource;
@property (nonatomic, copy) NSNumber * _Nullable userIndex;
@property (nonatomic, copy) NSNumber * _Nullable fabricIndex;
@property (nonatomic, copy) NSNumber * _Nullable sourceNode;
@property (nonatomic, copy) NSArray * _Nullable credentials;
@end

@interface MTRDoorLockClusterLockOperationErrorEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull lockOperationType;
@property (nonatomic, copy) NSNumber * _Nonnull operationSource;
@property (nonatomic, copy) NSNumber * _Nonnull operationError;
@property (nonatomic, copy) NSNumber * _Nullable userIndex;
@property (nonatomic, copy) NSNumber * _Nullable fabricIndex;
@property (nonatomic, copy) NSNumber * _Nullable sourceNode;
@property (nonatomic, copy) NSArray * _Nullable credentials;
@end

@interface MTRDoorLockClusterLockUserChangeEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull lockDataType;
@property (nonatomic, copy) NSNumber * _Nonnull dataOperationType;
@property (nonatomic, copy) NSNumber * _Nonnull operationSource;
@property (nonatomic, copy) NSNumber * _Nullable userIndex;
@property (nonatomic, copy) NSNumber * _Nullable fabricIndex;
@property (nonatomic, copy) NSNumber * _Nullable sourceNode;
@property (nonatomic, copy) NSNumber * _Nullable dataIndex;
@end

@interface MTRPumpConfigurationAndControlClusterSupplyVoltageLowEvent : NSObject <NSCopying>
@end

@interface MTRPumpConfigurationAndControlClusterSupplyVoltageHighEvent : NSObject <NSCopying>
@end

@interface MTRPumpConfigurationAndControlClusterPowerMissingPhaseEvent : NSObject <NSCopying>
@end

@interface MTRPumpConfigurationAndControlClusterSystemPressureLowEvent : NSObject <NSCopying>
@end

@interface MTRPumpConfigurationAndControlClusterSystemPressureHighEvent : NSObject <NSCopying>
@end

@interface MTRPumpConfigurationAndControlClusterDryRunningEvent : NSObject <NSCopying>
@end

@interface MTRPumpConfigurationAndControlClusterMotorTemperatureHighEvent : NSObject <NSCopying>
@end

@interface MTRPumpConfigurationAndControlClusterPumpMotorFatalFailureEvent : NSObject <NSCopying>
@end

@interface MTRPumpConfigurationAndControlClusterElectronicTemperatureHighEvent : NSObject <NSCopying>
@end

@interface MTRPumpConfigurationAndControlClusterPumpBlockedEvent : NSObject <NSCopying>
@end

@interface MTRPumpConfigurationAndControlClusterSensorFailureEvent : NSObject <NSCopying>
@end

@interface MTRPumpConfigurationAndControlClusterElectronicNonFatalFailureEvent : NSObject <NSCopying>
@end

@interface MTRPumpConfigurationAndControlClusterElectronicFatalFailureEvent : NSObject <NSCopying>
@end

@interface MTRPumpConfigurationAndControlClusterGeneralFaultEvent : NSObject <NSCopying>
@end

@interface MTRPumpConfigurationAndControlClusterLeakageEvent : NSObject <NSCopying>
@end

@interface MTRPumpConfigurationAndControlClusterAirDetectionEvent : NSObject <NSCopying>
@end

@interface MTRPumpConfigurationAndControlClusterTurbineOperationEvent : NSObject <NSCopying>
@end

@interface MTRThermostatClusterThermostatScheduleTransition : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull transitionTime;
@property (nonatomic, copy) NSNumber * _Nullable heatSetpoint;
@property (nonatomic, copy) NSNumber * _Nullable coolSetpoint;
@end

@interface MTRChannelClusterChannelInfo : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull majorNumber;
@property (nonatomic, copy) NSNumber * _Nonnull minorNumber;
@property (nonatomic, copy) NSString * _Nullable name;
@property (nonatomic, copy) NSString * _Nullable callSign;
@property (nonatomic, copy) NSString * _Nullable affiliateCallSign;
@end

@interface MTRChannelClusterLineupInfo : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull operatorName;
@property (nonatomic, copy) NSString * _Nullable lineupName;
@property (nonatomic, copy) NSString * _Nullable postalCode;
@property (nonatomic, copy) NSNumber * _Nonnull lineupInfoType;
@end

@interface MTRTargetNavigatorClusterTargetInfo : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull identifier;
@property (nonatomic, copy) NSString * _Nonnull name;
@end

@interface MTRMediaPlaybackClusterPlaybackPosition : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull updatedAt;
@property (nonatomic, copy) NSNumber * _Nullable position;
@end

@interface MTRMediaInputClusterInputInfo : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull index;
@property (nonatomic, copy) NSNumber * _Nonnull inputType;
@property (nonatomic, copy) NSString * _Nonnull name;
@property (nonatomic, copy) NSString * _Nonnull descriptionString;
@end

@interface MTRContentLauncherClusterDimension : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull width;
@property (nonatomic, copy) NSNumber * _Nonnull height;
@property (nonatomic, copy) NSNumber * _Nonnull metric;
@end

@interface MTRContentLauncherClusterAdditionalInfo : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull name;
@property (nonatomic, copy) NSString * _Nonnull value;
@end

@interface MTRContentLauncherClusterParameter : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull type;
@property (nonatomic, copy) NSString * _Nonnull value;
@property (nonatomic, copy) NSArray * _Nullable externalIDList;
@end

@interface MTRContentLauncherClusterContentSearch : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull parameterList;
@end

@interface MTRContentLauncherClusterStyleInformation : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nullable imageUrl;
@property (nonatomic, copy) NSString * _Nullable color;
@property (nonatomic, copy) MTRContentLauncherClusterDimension * _Nullable size;
@end

@interface MTRContentLauncherClusterBrandingInformation : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull providerName;
@property (nonatomic, copy) MTRContentLauncherClusterStyleInformation * _Nullable background;
@property (nonatomic, copy) MTRContentLauncherClusterStyleInformation * _Nullable logo;
@property (nonatomic, copy) MTRContentLauncherClusterStyleInformation * _Nullable progressBar;
@property (nonatomic, copy) MTRContentLauncherClusterStyleInformation * _Nullable splash;
@property (nonatomic, copy) MTRContentLauncherClusterStyleInformation * _Nullable waterMark;
@end

@interface MTRAudioOutputClusterOutputInfo : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull index;
@property (nonatomic, copy) NSNumber * _Nonnull outputType;
@property (nonatomic, copy) NSString * _Nonnull name;
@end

@interface MTRApplicationLauncherClusterApplication : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull catalogVendorId;
@property (nonatomic, copy) NSString * _Nonnull applicationId;
@end

@interface MTRApplicationLauncherClusterApplicationEP : NSObject <NSCopying>
@property (nonatomic, copy) MTRApplicationLauncherClusterApplication * _Nonnull application;
@property (nonatomic, copy) NSNumber * _Nullable endpoint;
@end

@interface MTRApplicationBasicClusterApplicationBasicApplication : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull catalogVendorId;
@property (nonatomic, copy) NSString * _Nonnull applicationId;
@end

@interface MTRClientMonitoringClusterMonitoringRegistration : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull clientNodeId;
@property (nonatomic, copy) NSNumber * _Nonnull iCid;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterSimpleStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull a;
@property (nonatomic, copy) NSNumber * _Nonnull b;
@property (nonatomic, copy) NSNumber * _Nonnull c;
@property (nonatomic, copy) NSData * _Nonnull d;
@property (nonatomic, copy) NSString * _Nonnull e;
@property (nonatomic, copy) NSNumber * _Nonnull f;
@property (nonatomic, copy) NSNumber * _Nonnull g;
@property (nonatomic, copy) NSNumber * _Nonnull h;
@end

MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterSimpleStruct")
@interface MTRTestClusterClusterSimpleStruct : MTRUnitTestingClusterSimpleStruct
@end
MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestFabricScoped : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull fabricSensitiveInt8u;
@property (nonatomic, copy) NSNumber * _Nullable optionalFabricSensitiveInt8u;
@property (nonatomic, copy) NSNumber * _Nullable nullableFabricSensitiveInt8u;
@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalFabricSensitiveInt8u;
@property (nonatomic, copy) NSString * _Nonnull fabricSensitiveCharString;
@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nonnull fabricSensitiveStruct;
@property (nonatomic, copy) NSArray * _Nonnull fabricSensitiveInt8uList;
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex;
@end

MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestFabricScoped")
@interface MTRTestClusterClusterTestFabricScoped : MTRUnitTestingClusterTestFabricScoped
@end
MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterNullablesAndOptionalsStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nullable nullableInt;
@property (nonatomic, copy) NSNumber * _Nullable optionalInt;
@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalInt;
@property (nonatomic, copy) NSString * _Nullable nullableString;
@property (nonatomic, copy) NSString * _Nullable optionalString;
@property (nonatomic, copy) NSString * _Nullable nullableOptionalString;
@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable nullableStruct;
@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable optionalStruct;
@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable nullableOptionalStruct;
@property (nonatomic, copy) NSArray * _Nullable nullableList;
@property (nonatomic, copy) NSArray * _Nullable optionalList;
@property (nonatomic, copy) NSArray * _Nullable nullableOptionalList;
@end

MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterNullablesAndOptionalsStruct")
@interface MTRTestClusterClusterNullablesAndOptionalsStruct : MTRUnitTestingClusterNullablesAndOptionalsStruct
@end
MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterNestedStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull a;
@property (nonatomic, copy) NSNumber * _Nonnull b;
@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nonnull c;
@end

MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterNestedStruct")
@interface MTRTestClusterClusterNestedStruct : MTRUnitTestingClusterNestedStruct
@end
MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterNestedStructList : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull a;
@property (nonatomic, copy) NSNumber * _Nonnull b;
@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nonnull c;
@property (nonatomic, copy) NSArray * _Nonnull d;
@property (nonatomic, copy) NSArray * _Nonnull e;
@property (nonatomic, copy) NSArray * _Nonnull f;
@property (nonatomic, copy) NSArray * _Nonnull g;
@end

MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterNestedStructList")
@interface MTRTestClusterClusterNestedStructList : MTRUnitTestingClusterNestedStructList
@end
MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterDoubleNestedStructList : NSObject <NSCopying>
@property (nonatomic, copy) NSArray * _Nonnull a;
@end

MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterDoubleNestedStructList")
@interface MTRTestClusterClusterDoubleNestedStructList : MTRUnitTestingClusterDoubleNestedStructList
@end
MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestListStructOctet : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull member1;
@property (nonatomic, copy) NSData * _Nonnull member2;
@end

MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestListStructOctet")
@interface MTRTestClusterClusterTestListStructOctet : MTRUnitTestingClusterTestListStructOctet
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestEventEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull arg1;
@property (nonatomic, copy) NSNumber * _Nonnull arg2;
@property (nonatomic, copy) NSNumber * _Nonnull arg3;
@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nonnull arg4;
@property (nonatomic, copy) NSArray * _Nonnull arg5;
@property (nonatomic, copy) NSArray * _Nonnull arg6;
@end

MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestEventEvent")
@interface MTRTestClusterClusterTestEventEvent : MTRUnitTestingClusterTestEventEvent
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestFabricScopedEventEvent : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex;
@end

MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestFabricScopedEventEvent")
@interface MTRTestClusterClusterTestFabricScopedEventEvent : MTRUnitTestingClusterTestFabricScopedEventEvent
@end

NS_ASSUME_NONNULL_END
