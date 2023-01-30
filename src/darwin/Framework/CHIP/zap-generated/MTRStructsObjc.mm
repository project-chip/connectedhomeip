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

#import "MTRStructsObjc.h"

NS_ASSUME_NONNULL_BEGIN

@implementation MTRScenesClusterAttributeValuePair
- (instancetype)init
{
    if (self = [super init]) {

        _attributeID = nil;

        _attributeValue = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRScenesClusterAttributeValuePair alloc] init];

    other.attributeID = self.attributeID;
    other.attributeValue = self.attributeValue;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: attributeID:%@; attributeValue:%@; >",
                                             NSStringFromClass([self class]), _attributeID, _attributeValue];
    return descriptionString;
}

- (void)setAttributeId:(NSNumber * _Nullable)attributeId
{
    self.attributeID = attributeId;
}

- (NSNumber * _Nullable)attributeId
{
    return self.attributeID;
}

@end

@implementation MTRScenesClusterExtensionFieldSet
- (instancetype)init
{
    if (self = [super init]) {

        _clusterID = @(0);

        _attributeValueList = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRScenesClusterExtensionFieldSet alloc] init];

    other.clusterID = self.clusterID;
    other.attributeValueList = self.attributeValueList;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: clusterID:%@; attributeValueList:%@; >",
                                             NSStringFromClass([self class]), _clusterID, _attributeValueList];
    return descriptionString;
}

- (void)setClusterId:(NSNumber * _Nonnull)clusterId
{
    self.clusterID = clusterId;
}

- (NSNumber * _Nonnull)clusterId
{
    return self.clusterID;
}

@end

@implementation MTRDescriptorClusterDeviceTypeStruct
- (instancetype)init
{
    if (self = [super init]) {

        _deviceType = @(0);

        _revision = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDescriptorClusterDeviceTypeStruct alloc] init];

    other.deviceType = self.deviceType;
    other.revision = self.revision;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: deviceType:%@; revision:%@; >", NSStringFromClass([self class]), _deviceType, _revision];
    return descriptionString;
}

- (void)setType:(NSNumber * _Nonnull)type
{
    self.deviceType = type;
}

- (NSNumber * _Nonnull)type
{
    return self.deviceType;
}

@end

@implementation MTRDescriptorClusterDeviceType : MTRDescriptorClusterDeviceTypeStruct
@end

@implementation MTRBindingClusterTargetStruct
- (instancetype)init
{
    if (self = [super init]) {

        _node = nil;

        _group = nil;

        _endpoint = nil;

        _cluster = nil;

        _fabricIndex = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRBindingClusterTargetStruct alloc] init];

    other.node = self.node;
    other.group = self.group;
    other.endpoint = self.endpoint;
    other.cluster = self.cluster;
    other.fabricIndex = self.fabricIndex;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: node:%@; group:%@; endpoint:%@; cluster:%@; fabricIndex:%@; >",
                                             NSStringFromClass([self class]), _node, _group, _endpoint, _cluster, _fabricIndex];
    return descriptionString;
}

@end

@implementation MTRAccessControlClusterTarget
- (instancetype)init
{
    if (self = [super init]) {

        _cluster = nil;

        _endpoint = nil;

        _deviceType = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRAccessControlClusterTarget alloc] init];

    other.cluster = self.cluster;
    other.endpoint = self.endpoint;
    other.deviceType = self.deviceType;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: cluster:%@; endpoint:%@; deviceType:%@; >",
                                             NSStringFromClass([self class]), _cluster, _endpoint, _deviceType];
    return descriptionString;
}

@end

@implementation MTRAccessControlClusterAccessControlEntryStruct
- (instancetype)init
{
    if (self = [super init]) {

        _privilege = @(0);

        _authMode = @(0);

        _subjects = nil;

        _targets = nil;

        _fabricIndex = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRAccessControlClusterAccessControlEntryStruct alloc] init];

    other.privilege = self.privilege;
    other.authMode = self.authMode;
    other.subjects = self.subjects;
    other.targets = self.targets;
    other.fabricIndex = self.fabricIndex;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: privilege:%@; authMode:%@; subjects:%@; targets:%@; fabricIndex:%@; >",
                  NSStringFromClass([self class]), _privilege, _authMode, _subjects, _targets, _fabricIndex];
    return descriptionString;
}

@end

@implementation MTRAccessControlClusterAccessControlEntry : MTRAccessControlClusterAccessControlEntryStruct
@end

@implementation MTRAccessControlClusterAccessControlExtensionStruct
- (instancetype)init
{
    if (self = [super init]) {

        _data = [NSData data];

        _fabricIndex = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRAccessControlClusterAccessControlExtensionStruct alloc] init];

    other.data = self.data;
    other.fabricIndex = self.fabricIndex;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: data:%@; fabricIndex:%@; >", NSStringFromClass([self class]),
                                             [_data base64EncodedStringWithOptions:0], _fabricIndex];
    return descriptionString;
}

@end

@implementation MTRAccessControlClusterExtensionEntry : MTRAccessControlClusterAccessControlExtensionStruct
@end

@implementation MTRAccessControlClusterAccessControlEntryChangedEvent
- (instancetype)init
{
    if (self = [super init]) {

        _adminNodeID = nil;

        _adminPasscodeID = nil;

        _changeType = @(0);

        _latestValue = nil;

        _fabricIndex = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRAccessControlClusterAccessControlEntryChangedEvent alloc] init];

    other.adminNodeID = self.adminNodeID;
    other.adminPasscodeID = self.adminPasscodeID;
    other.changeType = self.changeType;
    other.latestValue = self.latestValue;
    other.fabricIndex = self.fabricIndex;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: adminNodeID:%@; adminPasscodeID:%@; changeType:%@; latestValue:%@; fabricIndex:%@; >",
                  NSStringFromClass([self class]), _adminNodeID, _adminPasscodeID, _changeType, _latestValue, _fabricIndex];
    return descriptionString;
}

@end

@implementation MTRAccessControlClusterAccessControlExtensionChangedEvent
- (instancetype)init
{
    if (self = [super init]) {

        _adminNodeID = nil;

        _adminPasscodeID = nil;

        _changeType = @(0);

        _latestValue = nil;

        _fabricIndex = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRAccessControlClusterAccessControlExtensionChangedEvent alloc] init];

    other.adminNodeID = self.adminNodeID;
    other.adminPasscodeID = self.adminPasscodeID;
    other.changeType = self.changeType;
    other.latestValue = self.latestValue;
    other.fabricIndex = self.fabricIndex;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: adminNodeID:%@; adminPasscodeID:%@; changeType:%@; latestValue:%@; fabricIndex:%@; >",
                  NSStringFromClass([self class]), _adminNodeID, _adminPasscodeID, _changeType, _latestValue, _fabricIndex];
    return descriptionString;
}

@end

@implementation MTRActionsClusterActionStruct
- (instancetype)init
{
    if (self = [super init]) {

        _actionID = @(0);

        _name = @"";

        _type = @(0);

        _endpointListID = @(0);

        _supportedCommands = @(0);

        _state = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRActionsClusterActionStruct alloc] init];

    other.actionID = self.actionID;
    other.name = self.name;
    other.type = self.type;
    other.endpointListID = self.endpointListID;
    other.supportedCommands = self.supportedCommands;
    other.state = self.state;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: actionID:%@; name:%@; type:%@; endpointListID:%@; supportedCommands:%@; state:%@; >",
                  NSStringFromClass([self class]), _actionID, _name, _type, _endpointListID, _supportedCommands, _state];
    return descriptionString;
}

@end

@implementation MTRActionsClusterEndpointListStruct
- (instancetype)init
{
    if (self = [super init]) {

        _endpointListID = @(0);

        _name = @"";

        _type = @(0);

        _endpoints = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRActionsClusterEndpointListStruct alloc] init];

    other.endpointListID = self.endpointListID;
    other.name = self.name;
    other.type = self.type;
    other.endpoints = self.endpoints;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: endpointListID:%@; name:%@; type:%@; endpoints:%@; >",
                                             NSStringFromClass([self class]), _endpointListID, _name, _type, _endpoints];
    return descriptionString;
}

@end

@implementation MTRActionsClusterStateChangedEvent
- (instancetype)init
{
    if (self = [super init]) {

        _actionID = @(0);

        _invokeID = @(0);

        _newState = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRActionsClusterStateChangedEvent alloc] init];

    other.actionID = self.actionID;
    other.invokeID = self.invokeID;
    other.newState = self.newState;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: actionID:%@; invokeID:%@; newState:%@; >",
                                             NSStringFromClass([self class]), _actionID, _invokeID, _newState];
    return descriptionString;
}

@end

@implementation MTRActionsClusterActionFailedEvent
- (instancetype)init
{
    if (self = [super init]) {

        _actionID = @(0);

        _invokeID = @(0);

        _newState = @(0);

        _error = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRActionsClusterActionFailedEvent alloc] init];

    other.actionID = self.actionID;
    other.invokeID = self.invokeID;
    other.newState = self.newState;
    other.error = self.error;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: actionID:%@; invokeID:%@; newState:%@; error:%@; >",
                                             NSStringFromClass([self class]), _actionID, _invokeID, _newState, _error];
    return descriptionString;
}

@end

@implementation MTRBasicInformationClusterCapabilityMinimaStruct
- (instancetype)init
{
    if (self = [super init]) {

        _caseSessionsPerFabric = @(0);

        _subscriptionsPerFabric = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRBasicInformationClusterCapabilityMinimaStruct alloc] init];

    other.caseSessionsPerFabric = self.caseSessionsPerFabric;
    other.subscriptionsPerFabric = self.subscriptionsPerFabric;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: caseSessionsPerFabric:%@; subscriptionsPerFabric:%@; >",
                                             NSStringFromClass([self class]), _caseSessionsPerFabric, _subscriptionsPerFabric];
    return descriptionString;
}

@end

@implementation MTRBasicClusterCapabilityMinimaStruct : MTRBasicInformationClusterCapabilityMinimaStruct
@end

@implementation MTRBasicInformationClusterStartUpEvent
- (instancetype)init
{
    if (self = [super init]) {

        _softwareVersion = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRBasicInformationClusterStartUpEvent alloc] init];

    other.softwareVersion = self.softwareVersion;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: softwareVersion:%@; >", NSStringFromClass([self class]), _softwareVersion];
    return descriptionString;
}

@end

@implementation MTRBasicClusterStartUpEvent : MTRBasicInformationClusterStartUpEvent
@end

@implementation MTRBasicInformationClusterShutDownEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRBasicInformationClusterShutDownEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRBasicClusterShutDownEvent : MTRBasicInformationClusterShutDownEvent
@end

@implementation MTRBasicInformationClusterLeaveEvent
- (instancetype)init
{
    if (self = [super init]) {

        _fabricIndex = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRBasicInformationClusterLeaveEvent alloc] init];

    other.fabricIndex = self.fabricIndex;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: fabricIndex:%@; >", NSStringFromClass([self class]), _fabricIndex];
    return descriptionString;
}

@end

@implementation MTRBasicClusterLeaveEvent : MTRBasicInformationClusterLeaveEvent
@end

@implementation MTRBasicInformationClusterReachableChangedEvent
- (instancetype)init
{
    if (self = [super init]) {

        _reachableNewValue = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRBasicInformationClusterReachableChangedEvent alloc] init];

    other.reachableNewValue = self.reachableNewValue;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: reachableNewValue:%@; >", NSStringFromClass([self class]), _reachableNewValue];
    return descriptionString;
}

@end

@implementation MTRBasicClusterReachableChangedEvent : MTRBasicInformationClusterReachableChangedEvent
@end

@implementation MTROTASoftwareUpdateRequestorClusterProviderLocation
- (instancetype)init
{
    if (self = [super init]) {

        _providerNodeID = @(0);

        _endpoint = @(0);

        _fabricIndex = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTROTASoftwareUpdateRequestorClusterProviderLocation alloc] init];

    other.providerNodeID = self.providerNodeID;
    other.endpoint = self.endpoint;
    other.fabricIndex = self.fabricIndex;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: providerNodeID:%@; endpoint:%@; fabricIndex:%@; >",
                                             NSStringFromClass([self class]), _providerNodeID, _endpoint, _fabricIndex];
    return descriptionString;
}

@end

@implementation MTROtaSoftwareUpdateRequestorClusterProviderLocation : MTROTASoftwareUpdateRequestorClusterProviderLocation
@end

@implementation MTROTASoftwareUpdateRequestorClusterStateTransitionEvent
- (instancetype)init
{
    if (self = [super init]) {

        _previousState = @(0);

        _newState = @(0);

        _reason = @(0);

        _targetSoftwareVersion = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTROTASoftwareUpdateRequestorClusterStateTransitionEvent alloc] init];

    other.previousState = self.previousState;
    other.newState = self.newState;
    other.reason = self.reason;
    other.targetSoftwareVersion = self.targetSoftwareVersion;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: previousState:%@; newState:%@; reason:%@; targetSoftwareVersion:%@; >",
                  NSStringFromClass([self class]), _previousState, _newState, _reason, _targetSoftwareVersion];
    return descriptionString;
}

@end

@implementation MTROtaSoftwareUpdateRequestorClusterStateTransitionEvent : MTROTASoftwareUpdateRequestorClusterStateTransitionEvent
@end

@implementation MTROTASoftwareUpdateRequestorClusterVersionAppliedEvent
- (instancetype)init
{
    if (self = [super init]) {

        _softwareVersion = @(0);

        _productID = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTROTASoftwareUpdateRequestorClusterVersionAppliedEvent alloc] init];

    other.softwareVersion = self.softwareVersion;
    other.productID = self.productID;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: softwareVersion:%@; productID:%@; >",
                                             NSStringFromClass([self class]), _softwareVersion, _productID];
    return descriptionString;
}

@end

@implementation MTROtaSoftwareUpdateRequestorClusterVersionAppliedEvent : MTROTASoftwareUpdateRequestorClusterVersionAppliedEvent
@end

@implementation MTROTASoftwareUpdateRequestorClusterDownloadErrorEvent
- (instancetype)init
{
    if (self = [super init]) {

        _softwareVersion = @(0);

        _bytesDownloaded = @(0);

        _progressPercent = nil;

        _platformCode = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTROTASoftwareUpdateRequestorClusterDownloadErrorEvent alloc] init];

    other.softwareVersion = self.softwareVersion;
    other.bytesDownloaded = self.bytesDownloaded;
    other.progressPercent = self.progressPercent;
    other.platformCode = self.platformCode;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: softwareVersion:%@; bytesDownloaded:%@; progressPercent:%@; platformCode:%@; >",
                  NSStringFromClass([self class]), _softwareVersion, _bytesDownloaded, _progressPercent, _platformCode];
    return descriptionString;
}

@end

@implementation MTROtaSoftwareUpdateRequestorClusterDownloadErrorEvent : MTROTASoftwareUpdateRequestorClusterDownloadErrorEvent
@end

@implementation MTRPowerSourceClusterBatChargeFaultChangeType
- (instancetype)init
{
    if (self = [super init]) {

        _current = [NSArray array];

        _previous = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPowerSourceClusterBatChargeFaultChangeType alloc] init];

    other.current = self.current;
    other.previous = self.previous;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: current:%@; previous:%@; >", NSStringFromClass([self class]), _current, _previous];
    return descriptionString;
}

@end

@implementation MTRPowerSourceClusterBatFaultChangeType
- (instancetype)init
{
    if (self = [super init]) {

        _current = [NSArray array];

        _previous = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPowerSourceClusterBatFaultChangeType alloc] init];

    other.current = self.current;
    other.previous = self.previous;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: current:%@; previous:%@; >", NSStringFromClass([self class]), _current, _previous];
    return descriptionString;
}

@end

@implementation MTRPowerSourceClusterWiredFaultChangeType
- (instancetype)init
{
    if (self = [super init]) {

        _current = [NSArray array];

        _previous = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPowerSourceClusterWiredFaultChangeType alloc] init];

    other.current = self.current;
    other.previous = self.previous;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: current:%@; previous:%@; >", NSStringFromClass([self class]), _current, _previous];
    return descriptionString;
}

@end

@implementation MTRPowerSourceClusterWiredFaultChangeEvent
- (instancetype)init
{
    if (self = [super init]) {

        _current = [NSArray array];

        _previous = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPowerSourceClusterWiredFaultChangeEvent alloc] init];

    other.current = self.current;
    other.previous = self.previous;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: current:%@; previous:%@; >", NSStringFromClass([self class]), _current, _previous];
    return descriptionString;
}

@end

@implementation MTRPowerSourceClusterBatFaultChangeEvent
- (instancetype)init
{
    if (self = [super init]) {

        _current = [NSArray array];

        _previous = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPowerSourceClusterBatFaultChangeEvent alloc] init];

    other.current = self.current;
    other.previous = self.previous;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: current:%@; previous:%@; >", NSStringFromClass([self class]), _current, _previous];
    return descriptionString;
}

@end

@implementation MTRPowerSourceClusterBatChargeFaultChangeEvent
- (instancetype)init
{
    if (self = [super init]) {

        _current = [NSArray array];

        _previous = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPowerSourceClusterBatChargeFaultChangeEvent alloc] init];

    other.current = self.current;
    other.previous = self.previous;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: current:%@; previous:%@; >", NSStringFromClass([self class]), _current, _previous];
    return descriptionString;
}

@end

@implementation MTRGeneralCommissioningClusterBasicCommissioningInfo
- (instancetype)init
{
    if (self = [super init]) {

        _failSafeExpiryLengthSeconds = @(0);

        _maxCumulativeFailsafeSeconds = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRGeneralCommissioningClusterBasicCommissioningInfo alloc] init];

    other.failSafeExpiryLengthSeconds = self.failSafeExpiryLengthSeconds;
    other.maxCumulativeFailsafeSeconds = self.maxCumulativeFailsafeSeconds;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: failSafeExpiryLengthSeconds:%@; maxCumulativeFailsafeSeconds:%@; >",
                  NSStringFromClass([self class]), _failSafeExpiryLengthSeconds, _maxCumulativeFailsafeSeconds];
    return descriptionString;
}

@end

@implementation MTRNetworkCommissioningClusterNetworkInfo
- (instancetype)init
{
    if (self = [super init]) {

        _networkID = [NSData data];

        _connected = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRNetworkCommissioningClusterNetworkInfo alloc] init];

    other.networkID = self.networkID;
    other.connected = self.connected;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: networkID:%@; connected:%@; >", NSStringFromClass([self class]),
                  [_networkID base64EncodedStringWithOptions:0], _connected];
    return descriptionString;
}

@end

@implementation MTRNetworkCommissioningClusterThreadInterfaceScanResult
- (instancetype)init
{
    if (self = [super init]) {

        _panId = @(0);

        _extendedPanId = @(0);

        _networkName = @"";

        _channel = @(0);

        _version = @(0);

        _extendedAddress = [NSData data];

        _rssi = @(0);

        _lqi = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRNetworkCommissioningClusterThreadInterfaceScanResult alloc] init];

    other.panId = self.panId;
    other.extendedPanId = self.extendedPanId;
    other.networkName = self.networkName;
    other.channel = self.channel;
    other.version = self.version;
    other.extendedAddress = self.extendedAddress;
    other.rssi = self.rssi;
    other.lqi = self.lqi;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:
            @"<%@: panId:%@; extendedPanId:%@; networkName:%@; channel:%@; version:%@; extendedAddress:%@; rssi:%@; lqi:%@; >",
        NSStringFromClass([self class]), _panId, _extendedPanId, _networkName, _channel, _version,
        [_extendedAddress base64EncodedStringWithOptions:0], _rssi, _lqi];
    return descriptionString;
}

@end

@implementation MTRNetworkCommissioningClusterWiFiInterfaceScanResult
- (instancetype)init
{
    if (self = [super init]) {

        _security = @(0);

        _ssid = [NSData data];

        _bssid = [NSData data];

        _channel = @(0);

        _wiFiBand = @(0);

        _rssi = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRNetworkCommissioningClusterWiFiInterfaceScanResult alloc] init];

    other.security = self.security;
    other.ssid = self.ssid;
    other.bssid = self.bssid;
    other.channel = self.channel;
    other.wiFiBand = self.wiFiBand;
    other.rssi = self.rssi;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: security:%@; ssid:%@; bssid:%@; channel:%@; wiFiBand:%@; rssi:%@; >",
                  NSStringFromClass([self class]), _security, [_ssid base64EncodedStringWithOptions:0],
                  [_bssid base64EncodedStringWithOptions:0], _channel, _wiFiBand, _rssi];
    return descriptionString;
}

@end

@implementation MTRGeneralDiagnosticsClusterNetworkInterface
- (instancetype)init
{
    if (self = [super init]) {

        _name = @"";

        _isOperational = @(0);

        _offPremiseServicesReachableIPv4 = nil;

        _offPremiseServicesReachableIPv6 = nil;

        _hardwareAddress = [NSData data];

        _iPv4Addresses = [NSArray array];

        _iPv6Addresses = [NSArray array];

        _type = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRGeneralDiagnosticsClusterNetworkInterface alloc] init];

    other.name = self.name;
    other.isOperational = self.isOperational;
    other.offPremiseServicesReachableIPv4 = self.offPremiseServicesReachableIPv4;
    other.offPremiseServicesReachableIPv6 = self.offPremiseServicesReachableIPv6;
    other.hardwareAddress = self.hardwareAddress;
    other.iPv4Addresses = self.iPv4Addresses;
    other.iPv6Addresses = self.iPv6Addresses;
    other.type = self.type;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: name:%@; isOperational:%@; offPremiseServicesReachableIPv4:%@; offPremiseServicesReachableIPv6:%@; "
                         @"hardwareAddress:%@; iPv4Addresses:%@; iPv6Addresses:%@; type:%@; >",
        NSStringFromClass([self class]), _name, _isOperational, _offPremiseServicesReachableIPv4, _offPremiseServicesReachableIPv6,
        [_hardwareAddress base64EncodedStringWithOptions:0], _iPv4Addresses, _iPv6Addresses, _type];
    return descriptionString;
}

@end

@implementation MTRGeneralDiagnosticsClusterNetworkInterfaceType : MTRGeneralDiagnosticsClusterNetworkInterface
@end

@implementation MTRGeneralDiagnosticsClusterHardwareFaultChangeEvent
- (instancetype)init
{
    if (self = [super init]) {

        _current = [NSArray array];

        _previous = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRGeneralDiagnosticsClusterHardwareFaultChangeEvent alloc] init];

    other.current = self.current;
    other.previous = self.previous;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: current:%@; previous:%@; >", NSStringFromClass([self class]), _current, _previous];
    return descriptionString;
}

@end

@implementation MTRGeneralDiagnosticsClusterRadioFaultChangeEvent
- (instancetype)init
{
    if (self = [super init]) {

        _current = [NSArray array];

        _previous = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRGeneralDiagnosticsClusterRadioFaultChangeEvent alloc] init];

    other.current = self.current;
    other.previous = self.previous;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: current:%@; previous:%@; >", NSStringFromClass([self class]), _current, _previous];
    return descriptionString;
}

@end

@implementation MTRGeneralDiagnosticsClusterNetworkFaultChangeEvent
- (instancetype)init
{
    if (self = [super init]) {

        _current = [NSArray array];

        _previous = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRGeneralDiagnosticsClusterNetworkFaultChangeEvent alloc] init];

    other.current = self.current;
    other.previous = self.previous;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: current:%@; previous:%@; >", NSStringFromClass([self class]), _current, _previous];
    return descriptionString;
}

@end

@implementation MTRGeneralDiagnosticsClusterBootReasonEvent
- (instancetype)init
{
    if (self = [super init]) {

        _bootReason = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRGeneralDiagnosticsClusterBootReasonEvent alloc] init];

    other.bootReason = self.bootReason;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: bootReason:%@; >", NSStringFromClass([self class]), _bootReason];
    return descriptionString;
}

@end

@implementation MTRSoftwareDiagnosticsClusterThreadMetricsStruct
- (instancetype)init
{
    if (self = [super init]) {

        _id = @(0);

        _name = nil;

        _stackFreeCurrent = nil;

        _stackFreeMinimum = nil;

        _stackSize = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRSoftwareDiagnosticsClusterThreadMetricsStruct alloc] init];

    other.id = self.id;
    other.name = self.name;
    other.stackFreeCurrent = self.stackFreeCurrent;
    other.stackFreeMinimum = self.stackFreeMinimum;
    other.stackSize = self.stackSize;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: id:%@; name:%@; stackFreeCurrent:%@; stackFreeMinimum:%@; stackSize:%@; >",
                  NSStringFromClass([self class]), _id, _name, _stackFreeCurrent, _stackFreeMinimum, _stackSize];
    return descriptionString;
}

@end

@implementation MTRSoftwareDiagnosticsClusterThreadMetrics : MTRSoftwareDiagnosticsClusterThreadMetricsStruct
@end

@implementation MTRSoftwareDiagnosticsClusterSoftwareFaultEvent
- (instancetype)init
{
    if (self = [super init]) {

        _id = @(0);

        _name = nil;

        _faultRecording = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRSoftwareDiagnosticsClusterSoftwareFaultEvent alloc] init];

    other.id = self.id;
    other.name = self.name;
    other.faultRecording = self.faultRecording;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: id:%@; name:%@; faultRecording:%@; >", NSStringFromClass([self class]), _id, _name,
                  [_faultRecording base64EncodedStringWithOptions:0]];
    return descriptionString;
}

@end

@implementation MTRThreadNetworkDiagnosticsClusterNeighborTable
- (instancetype)init
{
    if (self = [super init]) {

        _extAddress = @(0);

        _age = @(0);

        _rloc16 = @(0);

        _linkFrameCounter = @(0);

        _mleFrameCounter = @(0);

        _lqi = @(0);

        _averageRssi = nil;

        _lastRssi = nil;

        _frameErrorRate = @(0);

        _messageErrorRate = @(0);

        _rxOnWhenIdle = @(0);

        _fullThreadDevice = @(0);

        _fullNetworkData = @(0);

        _isChild = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRThreadNetworkDiagnosticsClusterNeighborTable alloc] init];

    other.extAddress = self.extAddress;
    other.age = self.age;
    other.rloc16 = self.rloc16;
    other.linkFrameCounter = self.linkFrameCounter;
    other.mleFrameCounter = self.mleFrameCounter;
    other.lqi = self.lqi;
    other.averageRssi = self.averageRssi;
    other.lastRssi = self.lastRssi;
    other.frameErrorRate = self.frameErrorRate;
    other.messageErrorRate = self.messageErrorRate;
    other.rxOnWhenIdle = self.rxOnWhenIdle;
    other.fullThreadDevice = self.fullThreadDevice;
    other.fullNetworkData = self.fullNetworkData;
    other.isChild = self.isChild;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:
            @"<%@: extAddress:%@; age:%@; rloc16:%@; linkFrameCounter:%@; mleFrameCounter:%@; lqi:%@; averageRssi:%@; lastRssi:%@; "
            @"frameErrorRate:%@; messageErrorRate:%@; rxOnWhenIdle:%@; fullThreadDevice:%@; fullNetworkData:%@; isChild:%@; >",
        NSStringFromClass([self class]), _extAddress, _age, _rloc16, _linkFrameCounter, _mleFrameCounter, _lqi, _averageRssi,
        _lastRssi, _frameErrorRate, _messageErrorRate, _rxOnWhenIdle, _fullThreadDevice, _fullNetworkData, _isChild];
    return descriptionString;
}

@end

@implementation MTRThreadNetworkDiagnosticsClusterOperationalDatasetComponents
- (instancetype)init
{
    if (self = [super init]) {

        _activeTimestampPresent = @(0);

        _pendingTimestampPresent = @(0);

        _masterKeyPresent = @(0);

        _networkNamePresent = @(0);

        _extendedPanIdPresent = @(0);

        _meshLocalPrefixPresent = @(0);

        _delayPresent = @(0);

        _panIdPresent = @(0);

        _channelPresent = @(0);

        _pskcPresent = @(0);

        _securityPolicyPresent = @(0);

        _channelMaskPresent = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRThreadNetworkDiagnosticsClusterOperationalDatasetComponents alloc] init];

    other.activeTimestampPresent = self.activeTimestampPresent;
    other.pendingTimestampPresent = self.pendingTimestampPresent;
    other.masterKeyPresent = self.masterKeyPresent;
    other.networkNamePresent = self.networkNamePresent;
    other.extendedPanIdPresent = self.extendedPanIdPresent;
    other.meshLocalPrefixPresent = self.meshLocalPrefixPresent;
    other.delayPresent = self.delayPresent;
    other.panIdPresent = self.panIdPresent;
    other.channelPresent = self.channelPresent;
    other.pskcPresent = self.pskcPresent;
    other.securityPolicyPresent = self.securityPolicyPresent;
    other.channelMaskPresent = self.channelMaskPresent;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: activeTimestampPresent:%@; pendingTimestampPresent:%@; masterKeyPresent:%@; networkNamePresent:%@; "
                         @"extendedPanIdPresent:%@; meshLocalPrefixPresent:%@; delayPresent:%@; panIdPresent:%@; "
                         @"channelPresent:%@; pskcPresent:%@; securityPolicyPresent:%@; channelMaskPresent:%@; >",
        NSStringFromClass([self class]), _activeTimestampPresent, _pendingTimestampPresent, _masterKeyPresent, _networkNamePresent,
        _extendedPanIdPresent, _meshLocalPrefixPresent, _delayPresent, _panIdPresent, _channelPresent, _pskcPresent,
        _securityPolicyPresent, _channelMaskPresent];
    return descriptionString;
}

@end

@implementation MTRThreadNetworkDiagnosticsClusterRouteTable
- (instancetype)init
{
    if (self = [super init]) {

        _extAddress = @(0);

        _rloc16 = @(0);

        _routerId = @(0);

        _nextHop = @(0);

        _pathCost = @(0);

        _lqiIn = @(0);

        _lqiOut = @(0);

        _age = @(0);

        _allocated = @(0);

        _linkEstablished = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRThreadNetworkDiagnosticsClusterRouteTable alloc] init];

    other.extAddress = self.extAddress;
    other.rloc16 = self.rloc16;
    other.routerId = self.routerId;
    other.nextHop = self.nextHop;
    other.pathCost = self.pathCost;
    other.lqiIn = self.lqiIn;
    other.lqiOut = self.lqiOut;
    other.age = self.age;
    other.allocated = self.allocated;
    other.linkEstablished = self.linkEstablished;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: extAddress:%@; rloc16:%@; routerId:%@; nextHop:%@; pathCost:%@; lqiIn:%@; lqiOut:%@; "
                                   @"age:%@; allocated:%@; linkEstablished:%@; >",
                  NSStringFromClass([self class]), _extAddress, _rloc16, _routerId, _nextHop, _pathCost, _lqiIn, _lqiOut, _age,
                  _allocated, _linkEstablished];
    return descriptionString;
}

@end

@implementation MTRThreadNetworkDiagnosticsClusterSecurityPolicy
- (instancetype)init
{
    if (self = [super init]) {

        _rotationTime = @(0);

        _flags = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRThreadNetworkDiagnosticsClusterSecurityPolicy alloc] init];

    other.rotationTime = self.rotationTime;
    other.flags = self.flags;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: rotationTime:%@; flags:%@; >", NSStringFromClass([self class]), _rotationTime, _flags];
    return descriptionString;
}

@end

@implementation MTRThreadNetworkDiagnosticsClusterConnectionStatusEvent
- (instancetype)init
{
    if (self = [super init]) {

        _connectionStatus = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRThreadNetworkDiagnosticsClusterConnectionStatusEvent alloc] init];

    other.connectionStatus = self.connectionStatus;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: connectionStatus:%@; >", NSStringFromClass([self class]), _connectionStatus];
    return descriptionString;
}

@end

@implementation MTRThreadNetworkDiagnosticsClusterNetworkFaultChangeEvent
- (instancetype)init
{
    if (self = [super init]) {

        _current = [NSArray array];

        _previous = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRThreadNetworkDiagnosticsClusterNetworkFaultChangeEvent alloc] init];

    other.current = self.current;
    other.previous = self.previous;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: current:%@; previous:%@; >", NSStringFromClass([self class]), _current, _previous];
    return descriptionString;
}

@end

@implementation MTRWiFiNetworkDiagnosticsClusterDisconnectionEvent
- (instancetype)init
{
    if (self = [super init]) {

        _reasonCode = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRWiFiNetworkDiagnosticsClusterDisconnectionEvent alloc] init];

    other.reasonCode = self.reasonCode;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: reasonCode:%@; >", NSStringFromClass([self class]), _reasonCode];
    return descriptionString;
}

@end

@implementation MTRWiFiNetworkDiagnosticsClusterAssociationFailureEvent
- (instancetype)init
{
    if (self = [super init]) {

        _associationFailure = @(0);

        _status = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRWiFiNetworkDiagnosticsClusterAssociationFailureEvent alloc] init];

    other.associationFailure = self.associationFailure;
    other.status = self.status;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: associationFailure:%@; status:%@; >",
                                             NSStringFromClass([self class]), _associationFailure, _status];
    return descriptionString;
}

@end

@implementation MTRWiFiNetworkDiagnosticsClusterConnectionStatusEvent
- (instancetype)init
{
    if (self = [super init]) {

        _connectionStatus = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRWiFiNetworkDiagnosticsClusterConnectionStatusEvent alloc] init];

    other.connectionStatus = self.connectionStatus;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: connectionStatus:%@; >", NSStringFromClass([self class]), _connectionStatus];
    return descriptionString;
}

@end

@implementation MTRTimeSynchronizationClusterDstOffsetType
- (instancetype)init
{
    if (self = [super init]) {

        _offset = @(0);

        _validStarting = @(0);

        _validUntil = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRTimeSynchronizationClusterDstOffsetType alloc] init];

    other.offset = self.offset;
    other.validStarting = self.validStarting;
    other.validUntil = self.validUntil;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: offset:%@; validStarting:%@; validUntil:%@; >",
                                             NSStringFromClass([self class]), _offset, _validStarting, _validUntil];
    return descriptionString;
}

@end

@implementation MTRTimeSynchronizationClusterTimeZoneType
- (instancetype)init
{
    if (self = [super init]) {

        _offset = @(0);

        _validAt = @(0);

        _name = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRTimeSynchronizationClusterTimeZoneType alloc] init];

    other.offset = self.offset;
    other.validAt = self.validAt;
    other.name = self.name;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: offset:%@; validAt:%@; name:%@; >", NSStringFromClass([self class]), _offset, _validAt, _name];
    return descriptionString;
}

@end

@implementation MTRBridgedDeviceBasicInformationClusterStartUpEvent
- (instancetype)init
{
    if (self = [super init]) {

        _softwareVersion = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRBridgedDeviceBasicInformationClusterStartUpEvent alloc] init];

    other.softwareVersion = self.softwareVersion;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: softwareVersion:%@; >", NSStringFromClass([self class]), _softwareVersion];
    return descriptionString;
}

@end

@implementation MTRBridgedDeviceBasicClusterStartUpEvent : MTRBridgedDeviceBasicInformationClusterStartUpEvent
@end

@implementation MTRBridgedDeviceBasicInformationClusterShutDownEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRBridgedDeviceBasicInformationClusterShutDownEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRBridgedDeviceBasicClusterShutDownEvent : MTRBridgedDeviceBasicInformationClusterShutDownEvent
@end

@implementation MTRBridgedDeviceBasicInformationClusterLeaveEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRBridgedDeviceBasicInformationClusterLeaveEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRBridgedDeviceBasicClusterLeaveEvent : MTRBridgedDeviceBasicInformationClusterLeaveEvent
@end

@implementation MTRBridgedDeviceBasicInformationClusterReachableChangedEvent
- (instancetype)init
{
    if (self = [super init]) {

        _reachableNewValue = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRBridgedDeviceBasicInformationClusterReachableChangedEvent alloc] init];

    other.reachableNewValue = self.reachableNewValue;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: reachableNewValue:%@; >", NSStringFromClass([self class]), _reachableNewValue];
    return descriptionString;
}

@end

@implementation MTRBridgedDeviceBasicClusterReachableChangedEvent : MTRBridgedDeviceBasicInformationClusterReachableChangedEvent
@end

@implementation MTRSwitchClusterSwitchLatchedEvent
- (instancetype)init
{
    if (self = [super init]) {

        _newPosition = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRSwitchClusterSwitchLatchedEvent alloc] init];

    other.newPosition = self.newPosition;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: newPosition:%@; >", NSStringFromClass([self class]), _newPosition];
    return descriptionString;
}

@end

@implementation MTRSwitchClusterInitialPressEvent
- (instancetype)init
{
    if (self = [super init]) {

        _newPosition = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRSwitchClusterInitialPressEvent alloc] init];

    other.newPosition = self.newPosition;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: newPosition:%@; >", NSStringFromClass([self class]), _newPosition];
    return descriptionString;
}

@end

@implementation MTRSwitchClusterLongPressEvent
- (instancetype)init
{
    if (self = [super init]) {

        _newPosition = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRSwitchClusterLongPressEvent alloc] init];

    other.newPosition = self.newPosition;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: newPosition:%@; >", NSStringFromClass([self class]), _newPosition];
    return descriptionString;
}

@end

@implementation MTRSwitchClusterShortReleaseEvent
- (instancetype)init
{
    if (self = [super init]) {

        _previousPosition = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRSwitchClusterShortReleaseEvent alloc] init];

    other.previousPosition = self.previousPosition;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: previousPosition:%@; >", NSStringFromClass([self class]), _previousPosition];
    return descriptionString;
}

@end

@implementation MTRSwitchClusterLongReleaseEvent
- (instancetype)init
{
    if (self = [super init]) {

        _previousPosition = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRSwitchClusterLongReleaseEvent alloc] init];

    other.previousPosition = self.previousPosition;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: previousPosition:%@; >", NSStringFromClass([self class]), _previousPosition];
    return descriptionString;
}

@end

@implementation MTRSwitchClusterMultiPressOngoingEvent
- (instancetype)init
{
    if (self = [super init]) {

        _newPosition = @(0);

        _currentNumberOfPressesCounted = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRSwitchClusterMultiPressOngoingEvent alloc] init];

    other.newPosition = self.newPosition;
    other.currentNumberOfPressesCounted = self.currentNumberOfPressesCounted;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: newPosition:%@; currentNumberOfPressesCounted:%@; >",
                                             NSStringFromClass([self class]), _newPosition, _currentNumberOfPressesCounted];
    return descriptionString;
}

@end

@implementation MTRSwitchClusterMultiPressCompleteEvent
- (instancetype)init
{
    if (self = [super init]) {

        _previousPosition = @(0);

        _totalNumberOfPressesCounted = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRSwitchClusterMultiPressCompleteEvent alloc] init];

    other.previousPosition = self.previousPosition;
    other.totalNumberOfPressesCounted = self.totalNumberOfPressesCounted;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: previousPosition:%@; totalNumberOfPressesCounted:%@; >",
                                             NSStringFromClass([self class]), _previousPosition, _totalNumberOfPressesCounted];
    return descriptionString;
}

- (void)setNewPosition:(NSNumber * _Nonnull)newPosition
{
    self.previousPosition = newPosition;
}

- (NSNumber * _Nonnull)getNewPosition
{
    return self.previousPosition;
}
@end

@implementation MTROperationalCredentialsClusterFabricDescriptorStruct
- (instancetype)init
{
    if (self = [super init]) {

        _rootPublicKey = [NSData data];

        _vendorID = @(0);

        _fabricID = @(0);

        _nodeID = @(0);

        _label = @"";

        _fabricIndex = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTROperationalCredentialsClusterFabricDescriptorStruct alloc] init];

    other.rootPublicKey = self.rootPublicKey;
    other.vendorID = self.vendorID;
    other.fabricID = self.fabricID;
    other.nodeID = self.nodeID;
    other.label = self.label;
    other.fabricIndex = self.fabricIndex;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: rootPublicKey:%@; vendorID:%@; fabricID:%@; nodeID:%@; label:%@; fabricIndex:%@; >",
                  NSStringFromClass([self class]), [_rootPublicKey base64EncodedStringWithOptions:0], _vendorID, _fabricID, _nodeID,
                  _label, _fabricIndex];
    return descriptionString;
}

- (void)setVendorId:(NSNumber * _Nonnull)vendorId
{
    self.vendorID = vendorId;
}

- (NSNumber * _Nonnull)vendorId
{
    return self.vendorID;
}

- (void)setFabricId:(NSNumber * _Nonnull)fabricId
{
    self.fabricID = fabricId;
}

- (NSNumber * _Nonnull)fabricId
{
    return self.fabricID;
}

- (void)setNodeId:(NSNumber * _Nonnull)nodeId
{
    self.nodeID = nodeId;
}

- (NSNumber * _Nonnull)nodeId
{
    return self.nodeID;
}

@end

@implementation MTROperationalCredentialsClusterFabricDescriptor : MTROperationalCredentialsClusterFabricDescriptorStruct
@end

@implementation MTROperationalCredentialsClusterNOCStruct
- (instancetype)init
{
    if (self = [super init]) {

        _noc = [NSData data];

        _icac = nil;

        _fabricIndex = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTROperationalCredentialsClusterNOCStruct alloc] init];

    other.noc = self.noc;
    other.icac = self.icac;
    other.fabricIndex = self.fabricIndex;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: noc:%@; icac:%@; fabricIndex:%@; >", NSStringFromClass([self class]),
                  [_noc base64EncodedStringWithOptions:0], [_icac base64EncodedStringWithOptions:0], _fabricIndex];
    return descriptionString;
}

@end

@implementation MTRGroupKeyManagementClusterGroupInfoMapStruct
- (instancetype)init
{
    if (self = [super init]) {

        _groupId = @(0);

        _endpoints = [NSArray array];

        _groupName = nil;

        _fabricIndex = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRGroupKeyManagementClusterGroupInfoMapStruct alloc] init];

    other.groupId = self.groupId;
    other.endpoints = self.endpoints;
    other.groupName = self.groupName;
    other.fabricIndex = self.fabricIndex;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: groupId:%@; endpoints:%@; groupName:%@; fabricIndex:%@; >",
                                             NSStringFromClass([self class]), _groupId, _endpoints, _groupName, _fabricIndex];
    return descriptionString;
}

@end

@implementation MTRGroupKeyManagementClusterGroupKeyMapStruct
- (instancetype)init
{
    if (self = [super init]) {

        _groupId = @(0);

        _groupKeySetID = @(0);

        _fabricIndex = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRGroupKeyManagementClusterGroupKeyMapStruct alloc] init];

    other.groupId = self.groupId;
    other.groupKeySetID = self.groupKeySetID;
    other.fabricIndex = self.fabricIndex;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: groupId:%@; groupKeySetID:%@; fabricIndex:%@; >",
                                             NSStringFromClass([self class]), _groupId, _groupKeySetID, _fabricIndex];
    return descriptionString;
}

@end

@implementation MTRGroupKeyManagementClusterGroupKeySetStruct
- (instancetype)init
{
    if (self = [super init]) {

        _groupKeySetID = @(0);

        _groupKeySecurityPolicy = @(0);

        _epochKey0 = nil;

        _epochStartTime0 = nil;

        _epochKey1 = nil;

        _epochStartTime1 = nil;

        _epochKey2 = nil;

        _epochStartTime2 = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRGroupKeyManagementClusterGroupKeySetStruct alloc] init];

    other.groupKeySetID = self.groupKeySetID;
    other.groupKeySecurityPolicy = self.groupKeySecurityPolicy;
    other.epochKey0 = self.epochKey0;
    other.epochStartTime0 = self.epochStartTime0;
    other.epochKey1 = self.epochKey1;
    other.epochStartTime1 = self.epochStartTime1;
    other.epochKey2 = self.epochKey2;
    other.epochStartTime2 = self.epochStartTime2;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: groupKeySetID:%@; groupKeySecurityPolicy:%@; epochKey0:%@; epochStartTime0:%@; "
                                   @"epochKey1:%@; epochStartTime1:%@; epochKey2:%@; epochStartTime2:%@; >",
                  NSStringFromClass([self class]), _groupKeySetID, _groupKeySecurityPolicy,
                  [_epochKey0 base64EncodedStringWithOptions:0], _epochStartTime0, [_epochKey1 base64EncodedStringWithOptions:0],
                  _epochStartTime1, [_epochKey2 base64EncodedStringWithOptions:0], _epochStartTime2];
    return descriptionString;
}

@end

@implementation MTRFixedLabelClusterLabelStruct
- (instancetype)init
{
    if (self = [super init]) {

        _label = @"";

        _value = @"";
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRFixedLabelClusterLabelStruct alloc] init];

    other.label = self.label;
    other.value = self.value;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: label:%@; value:%@; >", NSStringFromClass([self class]), _label, _value];
    return descriptionString;
}

@end

@implementation MTRUserLabelClusterLabelStruct
- (instancetype)init
{
    if (self = [super init]) {

        _label = @"";

        _value = @"";
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRUserLabelClusterLabelStruct alloc] init];

    other.label = self.label;
    other.value = self.value;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: label:%@; value:%@; >", NSStringFromClass([self class]), _label, _value];
    return descriptionString;
}

@end

@implementation MTRBooleanStateClusterStateChangeEvent
- (instancetype)init
{
    if (self = [super init]) {

        _stateValue = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRBooleanStateClusterStateChangeEvent alloc] init];

    other.stateValue = self.stateValue;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: stateValue:%@; >", NSStringFromClass([self class]), _stateValue];
    return descriptionString;
}

@end

@implementation MTRModeSelectClusterSemanticTagStruct
- (instancetype)init
{
    if (self = [super init]) {

        _mfgCode = @(0);

        _value = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRModeSelectClusterSemanticTagStruct alloc] init];

    other.mfgCode = self.mfgCode;
    other.value = self.value;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: mfgCode:%@; value:%@; >", NSStringFromClass([self class]), _mfgCode, _value];
    return descriptionString;
}

@end

@implementation MTRModeSelectClusterSemanticTag : MTRModeSelectClusterSemanticTagStruct
@end

@implementation MTRModeSelectClusterModeOptionStruct
- (instancetype)init
{
    if (self = [super init]) {

        _label = @"";

        _mode = @(0);

        _semanticTags = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRModeSelectClusterModeOptionStruct alloc] init];

    other.label = self.label;
    other.mode = self.mode;
    other.semanticTags = self.semanticTags;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: label:%@; mode:%@; semanticTags:%@; >",
                                             NSStringFromClass([self class]), _label, _mode, _semanticTags];
    return descriptionString;
}

@end

@implementation MTRDoorLockClusterCredentialStruct
- (instancetype)init
{
    if (self = [super init]) {

        _credentialType = @(0);

        _credentialIndex = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDoorLockClusterCredentialStruct alloc] init];

    other.credentialType = self.credentialType;
    other.credentialIndex = self.credentialIndex;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: credentialType:%@; credentialIndex:%@; >",
                                             NSStringFromClass([self class]), _credentialType, _credentialIndex];
    return descriptionString;
}

@end

@implementation MTRDoorLockClusterDlCredential : MTRDoorLockClusterCredentialStruct
@end

@implementation MTRDoorLockClusterDoorLockAlarmEvent
- (instancetype)init
{
    if (self = [super init]) {

        _alarmCode = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDoorLockClusterDoorLockAlarmEvent alloc] init];

    other.alarmCode = self.alarmCode;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: alarmCode:%@; >", NSStringFromClass([self class]), _alarmCode];
    return descriptionString;
}

@end

@implementation MTRDoorLockClusterDoorStateChangeEvent
- (instancetype)init
{
    if (self = [super init]) {

        _doorState = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDoorLockClusterDoorStateChangeEvent alloc] init];

    other.doorState = self.doorState;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: doorState:%@; >", NSStringFromClass([self class]), _doorState];
    return descriptionString;
}

@end

@implementation MTRDoorLockClusterLockOperationEvent
- (instancetype)init
{
    if (self = [super init]) {

        _lockOperationType = @(0);

        _operationSource = @(0);

        _userIndex = nil;

        _fabricIndex = nil;

        _sourceNode = nil;

        _credentials = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDoorLockClusterLockOperationEvent alloc] init];

    other.lockOperationType = self.lockOperationType;
    other.operationSource = self.operationSource;
    other.userIndex = self.userIndex;
    other.fabricIndex = self.fabricIndex;
    other.sourceNode = self.sourceNode;
    other.credentials = self.credentials;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:
            @"<%@: lockOperationType:%@; operationSource:%@; userIndex:%@; fabricIndex:%@; sourceNode:%@; credentials:%@; >",
        NSStringFromClass([self class]), _lockOperationType, _operationSource, _userIndex, _fabricIndex, _sourceNode, _credentials];
    return descriptionString;
}

@end

@implementation MTRDoorLockClusterLockOperationErrorEvent
- (instancetype)init
{
    if (self = [super init]) {

        _lockOperationType = @(0);

        _operationSource = @(0);

        _operationError = @(0);

        _userIndex = nil;

        _fabricIndex = nil;

        _sourceNode = nil;

        _credentials = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDoorLockClusterLockOperationErrorEvent alloc] init];

    other.lockOperationType = self.lockOperationType;
    other.operationSource = self.operationSource;
    other.operationError = self.operationError;
    other.userIndex = self.userIndex;
    other.fabricIndex = self.fabricIndex;
    other.sourceNode = self.sourceNode;
    other.credentials = self.credentials;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: lockOperationType:%@; operationSource:%@; operationError:%@; "
                                                              @"userIndex:%@; fabricIndex:%@; sourceNode:%@; credentials:%@; >",
                                             NSStringFromClass([self class]), _lockOperationType, _operationSource, _operationError,
                                             _userIndex, _fabricIndex, _sourceNode, _credentials];
    return descriptionString;
}

@end

@implementation MTRDoorLockClusterLockUserChangeEvent
- (instancetype)init
{
    if (self = [super init]) {

        _lockDataType = @(0);

        _dataOperationType = @(0);

        _operationSource = @(0);

        _userIndex = nil;

        _fabricIndex = nil;

        _sourceNode = nil;

        _dataIndex = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDoorLockClusterLockUserChangeEvent alloc] init];

    other.lockDataType = self.lockDataType;
    other.dataOperationType = self.dataOperationType;
    other.operationSource = self.operationSource;
    other.userIndex = self.userIndex;
    other.fabricIndex = self.fabricIndex;
    other.sourceNode = self.sourceNode;
    other.dataIndex = self.dataIndex;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: lockDataType:%@; dataOperationType:%@; operationSource:%@; "
                                                              @"userIndex:%@; fabricIndex:%@; sourceNode:%@; dataIndex:%@; >",
                                             NSStringFromClass([self class]), _lockDataType, _dataOperationType, _operationSource,
                                             _userIndex, _fabricIndex, _sourceNode, _dataIndex];
    return descriptionString;
}

@end

@implementation MTRPumpConfigurationAndControlClusterSupplyVoltageLowEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPumpConfigurationAndControlClusterSupplyVoltageLowEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRPumpConfigurationAndControlClusterSupplyVoltageHighEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPumpConfigurationAndControlClusterSupplyVoltageHighEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRPumpConfigurationAndControlClusterPowerMissingPhaseEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPumpConfigurationAndControlClusterPowerMissingPhaseEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRPumpConfigurationAndControlClusterSystemPressureLowEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPumpConfigurationAndControlClusterSystemPressureLowEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRPumpConfigurationAndControlClusterSystemPressureHighEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPumpConfigurationAndControlClusterSystemPressureHighEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRPumpConfigurationAndControlClusterDryRunningEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPumpConfigurationAndControlClusterDryRunningEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRPumpConfigurationAndControlClusterMotorTemperatureHighEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPumpConfigurationAndControlClusterMotorTemperatureHighEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRPumpConfigurationAndControlClusterPumpMotorFatalFailureEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPumpConfigurationAndControlClusterPumpMotorFatalFailureEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRPumpConfigurationAndControlClusterElectronicTemperatureHighEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPumpConfigurationAndControlClusterElectronicTemperatureHighEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRPumpConfigurationAndControlClusterPumpBlockedEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPumpConfigurationAndControlClusterPumpBlockedEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRPumpConfigurationAndControlClusterSensorFailureEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPumpConfigurationAndControlClusterSensorFailureEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRPumpConfigurationAndControlClusterElectronicNonFatalFailureEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPumpConfigurationAndControlClusterElectronicNonFatalFailureEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRPumpConfigurationAndControlClusterElectronicFatalFailureEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPumpConfigurationAndControlClusterElectronicFatalFailureEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRPumpConfigurationAndControlClusterGeneralFaultEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPumpConfigurationAndControlClusterGeneralFaultEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRPumpConfigurationAndControlClusterLeakageEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPumpConfigurationAndControlClusterLeakageEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRPumpConfigurationAndControlClusterAirDetectionEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPumpConfigurationAndControlClusterAirDetectionEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRPumpConfigurationAndControlClusterTurbineOperationEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPumpConfigurationAndControlClusterTurbineOperationEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRThermostatClusterThermostatScheduleTransition
- (instancetype)init
{
    if (self = [super init]) {

        _transitionTime = @(0);

        _heatSetpoint = nil;

        _coolSetpoint = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRThermostatClusterThermostatScheduleTransition alloc] init];

    other.transitionTime = self.transitionTime;
    other.heatSetpoint = self.heatSetpoint;
    other.coolSetpoint = self.coolSetpoint;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: transitionTime:%@; heatSetpoint:%@; coolSetpoint:%@; >",
                                             NSStringFromClass([self class]), _transitionTime, _heatSetpoint, _coolSetpoint];
    return descriptionString;
}

@end

@implementation MTRChannelClusterChannelInfoStruct
- (instancetype)init
{
    if (self = [super init]) {

        _majorNumber = @(0);

        _minorNumber = @(0);

        _name = nil;

        _callSign = nil;

        _affiliateCallSign = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRChannelClusterChannelInfoStruct alloc] init];

    other.majorNumber = self.majorNumber;
    other.minorNumber = self.minorNumber;
    other.name = self.name;
    other.callSign = self.callSign;
    other.affiliateCallSign = self.affiliateCallSign;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: majorNumber:%@; minorNumber:%@; name:%@; callSign:%@; affiliateCallSign:%@; >",
                  NSStringFromClass([self class]), _majorNumber, _minorNumber, _name, _callSign, _affiliateCallSign];
    return descriptionString;
}

@end

@implementation MTRChannelClusterChannelInfo : MTRChannelClusterChannelInfoStruct
@end

@implementation MTRChannelClusterLineupInfoStruct
- (instancetype)init
{
    if (self = [super init]) {

        _operatorName = @"";

        _lineupName = nil;

        _postalCode = nil;

        _lineupInfoType = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRChannelClusterLineupInfoStruct alloc] init];

    other.operatorName = self.operatorName;
    other.lineupName = self.lineupName;
    other.postalCode = self.postalCode;
    other.lineupInfoType = self.lineupInfoType;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: operatorName:%@; lineupName:%@; postalCode:%@; lineupInfoType:%@; >",
                  NSStringFromClass([self class]), _operatorName, _lineupName, _postalCode, _lineupInfoType];
    return descriptionString;
}

@end

@implementation MTRChannelClusterLineupInfo : MTRChannelClusterLineupInfoStruct
@end

@implementation MTRTargetNavigatorClusterTargetInfoStruct
- (instancetype)init
{
    if (self = [super init]) {

        _identifier = @(0);

        _name = @"";
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRTargetNavigatorClusterTargetInfoStruct alloc] init];

    other.identifier = self.identifier;
    other.name = self.name;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: identifier:%@; name:%@; >", NSStringFromClass([self class]), _identifier, _name];
    return descriptionString;
}

@end

@implementation MTRTargetNavigatorClusterTargetInfo : MTRTargetNavigatorClusterTargetInfoStruct
@end

@implementation MTRMediaPlaybackClusterPlaybackPositionStruct
- (instancetype)init
{
    if (self = [super init]) {

        _updatedAt = @(0);

        _position = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRMediaPlaybackClusterPlaybackPositionStruct alloc] init];

    other.updatedAt = self.updatedAt;
    other.position = self.position;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: updatedAt:%@; position:%@; >", NSStringFromClass([self class]), _updatedAt, _position];
    return descriptionString;
}

@end

@implementation MTRMediaPlaybackClusterPlaybackPosition : MTRMediaPlaybackClusterPlaybackPositionStruct
@end

@implementation MTRMediaInputClusterInputInfoStruct
- (instancetype)init
{
    if (self = [super init]) {

        _index = @(0);

        _inputType = @(0);

        _name = @"";

        _descriptionString = @"";
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRMediaInputClusterInputInfoStruct alloc] init];

    other.index = self.index;
    other.inputType = self.inputType;
    other.name = self.name;
    other.descriptionString = self.descriptionString;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: index:%@; inputType:%@; name:%@; descriptionString:%@; >",
                                             NSStringFromClass([self class]), _index, _inputType, _name, _descriptionString];
    return descriptionString;
}

@end

@implementation MTRMediaInputClusterInputInfo : MTRMediaInputClusterInputInfoStruct
@end

@implementation MTRContentLauncherClusterDimensionStruct
- (instancetype)init
{
    if (self = [super init]) {

        _width = @(0);

        _height = @(0);

        _metric = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRContentLauncherClusterDimensionStruct alloc] init];

    other.width = self.width;
    other.height = self.height;
    other.metric = self.metric;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: width:%@; height:%@; metric:%@; >", NSStringFromClass([self class]), _width, _height, _metric];
    return descriptionString;
}

@end

@implementation MTRContentLauncherClusterDimension : MTRContentLauncherClusterDimensionStruct
@end

@implementation MTRContentLauncherClusterAdditionalInfoStruct
- (instancetype)init
{
    if (self = [super init]) {

        _name = @"";

        _value = @"";
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRContentLauncherClusterAdditionalInfoStruct alloc] init];

    other.name = self.name;
    other.value = self.value;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: name:%@; value:%@; >", NSStringFromClass([self class]), _name, _value];
    return descriptionString;
}

@end

@implementation MTRContentLauncherClusterAdditionalInfo : MTRContentLauncherClusterAdditionalInfoStruct
@end

@implementation MTRContentLauncherClusterParameterStruct
- (instancetype)init
{
    if (self = [super init]) {

        _type = @(0);

        _value = @"";

        _externalIDList = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRContentLauncherClusterParameterStruct alloc] init];

    other.type = self.type;
    other.value = self.value;
    other.externalIDList = self.externalIDList;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: type:%@; value:%@; externalIDList:%@; >",
                                             NSStringFromClass([self class]), _type, _value, _externalIDList];
    return descriptionString;
}

@end

@implementation MTRContentLauncherClusterParameter : MTRContentLauncherClusterParameterStruct
@end

@implementation MTRContentLauncherClusterContentSearchStruct
- (instancetype)init
{
    if (self = [super init]) {

        _parameterList = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRContentLauncherClusterContentSearchStruct alloc] init];

    other.parameterList = self.parameterList;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: parameterList:%@; >", NSStringFromClass([self class]), _parameterList];
    return descriptionString;
}

@end

@implementation MTRContentLauncherClusterContentSearch : MTRContentLauncherClusterContentSearchStruct
@end

@implementation MTRContentLauncherClusterStyleInformationStruct
- (instancetype)init
{
    if (self = [super init]) {

        _imageURL = nil;

        _color = nil;

        _size = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRContentLauncherClusterStyleInformationStruct alloc] init];

    other.imageURL = self.imageURL;
    other.color = self.color;
    other.size = self.size;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: imageURL:%@; color:%@; size:%@; >", NSStringFromClass([self class]), _imageURL, _color, _size];
    return descriptionString;
}

- (void)setImageUrl:(NSString * _Nullable)imageUrl
{
    self.imageURL = imageUrl;
}

- (NSString * _Nullable)imageUrl
{
    return self.imageURL;
}

@end

@implementation MTRContentLauncherClusterStyleInformation : MTRContentLauncherClusterStyleInformationStruct
@end

@implementation MTRContentLauncherClusterBrandingInformationStruct
- (instancetype)init
{
    if (self = [super init]) {

        _providerName = @"";

        _background = nil;

        _logo = nil;

        _progressBar = nil;

        _splash = nil;

        _waterMark = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRContentLauncherClusterBrandingInformationStruct alloc] init];

    other.providerName = self.providerName;
    other.background = self.background;
    other.logo = self.logo;
    other.progressBar = self.progressBar;
    other.splash = self.splash;
    other.waterMark = self.waterMark;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: providerName:%@; background:%@; logo:%@; progressBar:%@; splash:%@; waterMark:%@; >",
                  NSStringFromClass([self class]), _providerName, _background, _logo, _progressBar, _splash, _waterMark];
    return descriptionString;
}

@end

@implementation MTRContentLauncherClusterBrandingInformation : MTRContentLauncherClusterBrandingInformationStruct
@end

@implementation MTRAudioOutputClusterOutputInfoStruct
- (instancetype)init
{
    if (self = [super init]) {

        _index = @(0);

        _outputType = @(0);

        _name = @"";
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRAudioOutputClusterOutputInfoStruct alloc] init];

    other.index = self.index;
    other.outputType = self.outputType;
    other.name = self.name;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: index:%@; outputType:%@; name:%@; >", NSStringFromClass([self class]), _index, _outputType, _name];
    return descriptionString;
}

@end

@implementation MTRAudioOutputClusterOutputInfo : MTRAudioOutputClusterOutputInfoStruct
@end

@implementation MTRApplicationLauncherClusterApplicationStruct
- (instancetype)init
{
    if (self = [super init]) {

        _catalogVendorID = @(0);

        _applicationID = @"";
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRApplicationLauncherClusterApplicationStruct alloc] init];

    other.catalogVendorID = self.catalogVendorID;
    other.applicationID = self.applicationID;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: catalogVendorID:%@; applicationID:%@; >",
                                             NSStringFromClass([self class]), _catalogVendorID, _applicationID];
    return descriptionString;
}

- (void)setCatalogVendorId:(NSNumber * _Nonnull)catalogVendorId
{
    self.catalogVendorID = catalogVendorId;
}

- (NSNumber * _Nonnull)catalogVendorId
{
    return self.catalogVendorID;
}

- (void)setApplicationId:(NSString * _Nonnull)applicationId
{
    self.applicationID = applicationId;
}

- (NSString * _Nonnull)applicationId
{
    return self.applicationID;
}

@end

@implementation MTRApplicationLauncherClusterApplication : MTRApplicationLauncherClusterApplicationStruct
@end

@implementation MTRApplicationLauncherClusterApplicationEPStruct
- (instancetype)init
{
    if (self = [super init]) {

        _application = [MTRApplicationLauncherClusterApplicationStruct new];

        _endpoint = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRApplicationLauncherClusterApplicationEPStruct alloc] init];

    other.application = self.application;
    other.endpoint = self.endpoint;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: application:%@; endpoint:%@; >", NSStringFromClass([self class]), _application, _endpoint];
    return descriptionString;
}

@end

@implementation MTRApplicationLauncherClusterApplicationEP : MTRApplicationLauncherClusterApplicationEPStruct
@end

@implementation MTRApplicationBasicClusterApplicationStruct
- (instancetype)init
{
    if (self = [super init]) {

        _catalogVendorID = @(0);

        _applicationID = @"";
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRApplicationBasicClusterApplicationStruct alloc] init];

    other.catalogVendorID = self.catalogVendorID;
    other.applicationID = self.applicationID;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: catalogVendorID:%@; applicationID:%@; >",
                                             NSStringFromClass([self class]), _catalogVendorID, _applicationID];
    return descriptionString;
}

- (void)setCatalogVendorId:(NSNumber * _Nonnull)catalogVendorId
{
    self.catalogVendorID = catalogVendorId;
}

- (NSNumber * _Nonnull)catalogVendorId
{
    return self.catalogVendorID;
}

- (void)setApplicationId:(NSString * _Nonnull)applicationId
{
    self.applicationID = applicationId;
}

- (NSString * _Nonnull)applicationId
{
    return self.applicationID;
}

@end

@implementation MTRApplicationBasicClusterApplicationBasicApplication : MTRApplicationBasicClusterApplicationStruct
@end

@implementation MTRUnitTestingClusterSimpleStruct
- (instancetype)init
{
    if (self = [super init]) {

        _a = @(0);

        _b = @(0);

        _c = @(0);

        _d = [NSData data];

        _e = @"";

        _f = @(0);

        _g = @(0);

        _h = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRUnitTestingClusterSimpleStruct alloc] init];

    other.a = self.a;
    other.b = self.b;
    other.c = self.c;
    other.d = self.d;
    other.e = self.e;
    other.f = self.f;
    other.g = self.g;
    other.h = self.h;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: a:%@; b:%@; c:%@; d:%@; e:%@; f:%@; g:%@; h:%@; >", NSStringFromClass([self class]), _a,
                  _b, _c, [_d base64EncodedStringWithOptions:0], _e, _f, _g, _h];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterSimpleStruct : MTRUnitTestingClusterSimpleStruct
@end

@implementation MTRUnitTestingClusterTestFabricScoped
- (instancetype)init
{
    if (self = [super init]) {

        _fabricSensitiveInt8u = @(0);

        _optionalFabricSensitiveInt8u = nil;

        _nullableFabricSensitiveInt8u = nil;

        _nullableOptionalFabricSensitiveInt8u = nil;

        _fabricSensitiveCharString = @"";

        _fabricSensitiveStruct = [MTRUnitTestingClusterSimpleStruct new];

        _fabricSensitiveInt8uList = [NSArray array];

        _fabricIndex = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRUnitTestingClusterTestFabricScoped alloc] init];

    other.fabricSensitiveInt8u = self.fabricSensitiveInt8u;
    other.optionalFabricSensitiveInt8u = self.optionalFabricSensitiveInt8u;
    other.nullableFabricSensitiveInt8u = self.nullableFabricSensitiveInt8u;
    other.nullableOptionalFabricSensitiveInt8u = self.nullableOptionalFabricSensitiveInt8u;
    other.fabricSensitiveCharString = self.fabricSensitiveCharString;
    other.fabricSensitiveStruct = self.fabricSensitiveStruct;
    other.fabricSensitiveInt8uList = self.fabricSensitiveInt8uList;
    other.fabricIndex = self.fabricIndex;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: fabricSensitiveInt8u:%@; optionalFabricSensitiveInt8u:%@; nullableFabricSensitiveInt8u:%@; "
                         @"nullableOptionalFabricSensitiveInt8u:%@; fabricSensitiveCharString:%@; fabricSensitiveStruct:%@; "
                         @"fabricSensitiveInt8uList:%@; fabricIndex:%@; >",
        NSStringFromClass([self class]), _fabricSensitiveInt8u, _optionalFabricSensitiveInt8u, _nullableFabricSensitiveInt8u,
        _nullableOptionalFabricSensitiveInt8u, _fabricSensitiveCharString, _fabricSensitiveStruct, _fabricSensitiveInt8uList,
        _fabricIndex];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterTestFabricScoped : MTRUnitTestingClusterTestFabricScoped
@end

@implementation MTRUnitTestingClusterNullablesAndOptionalsStruct
- (instancetype)init
{
    if (self = [super init]) {

        _nullableInt = nil;

        _optionalInt = nil;

        _nullableOptionalInt = nil;

        _nullableString = nil;

        _optionalString = nil;

        _nullableOptionalString = nil;

        _nullableStruct = nil;

        _optionalStruct = nil;

        _nullableOptionalStruct = nil;

        _nullableList = nil;

        _optionalList = nil;

        _nullableOptionalList = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRUnitTestingClusterNullablesAndOptionalsStruct alloc] init];

    other.nullableInt = self.nullableInt;
    other.optionalInt = self.optionalInt;
    other.nullableOptionalInt = self.nullableOptionalInt;
    other.nullableString = self.nullableString;
    other.optionalString = self.optionalString;
    other.nullableOptionalString = self.nullableOptionalString;
    other.nullableStruct = self.nullableStruct;
    other.optionalStruct = self.optionalStruct;
    other.nullableOptionalStruct = self.nullableOptionalStruct;
    other.nullableList = self.nullableList;
    other.optionalList = self.optionalList;
    other.nullableOptionalList = self.nullableOptionalList;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: nullableInt:%@; optionalInt:%@; nullableOptionalInt:%@; nullableString:%@; "
                                   @"optionalString:%@; nullableOptionalString:%@; nullableStruct:%@; optionalStruct:%@; "
                                   @"nullableOptionalStruct:%@; nullableList:%@; optionalList:%@; nullableOptionalList:%@; >",
                  NSStringFromClass([self class]), _nullableInt, _optionalInt, _nullableOptionalInt, _nullableString,
                  _optionalString, _nullableOptionalString, _nullableStruct, _optionalStruct, _nullableOptionalStruct,
                  _nullableList, _optionalList, _nullableOptionalList];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterNullablesAndOptionalsStruct : MTRUnitTestingClusterNullablesAndOptionalsStruct
@end

@implementation MTRUnitTestingClusterNestedStruct
- (instancetype)init
{
    if (self = [super init]) {

        _a = @(0);

        _b = @(0);

        _c = [MTRUnitTestingClusterSimpleStruct new];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRUnitTestingClusterNestedStruct alloc] init];

    other.a = self.a;
    other.b = self.b;
    other.c = self.c;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: a:%@; b:%@; c:%@; >", NSStringFromClass([self class]), _a, _b, _c];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterNestedStruct : MTRUnitTestingClusterNestedStruct
@end

@implementation MTRUnitTestingClusterNestedStructList
- (instancetype)init
{
    if (self = [super init]) {

        _a = @(0);

        _b = @(0);

        _c = [MTRUnitTestingClusterSimpleStruct new];

        _d = [NSArray array];

        _e = [NSArray array];

        _f = [NSArray array];

        _g = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRUnitTestingClusterNestedStructList alloc] init];

    other.a = self.a;
    other.b = self.b;
    other.c = self.c;
    other.d = self.d;
    other.e = self.e;
    other.f = self.f;
    other.g = self.g;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: a:%@; b:%@; c:%@; d:%@; e:%@; f:%@; g:%@; >",
                                             NSStringFromClass([self class]), _a, _b, _c, _d, _e, _f, _g];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterNestedStructList : MTRUnitTestingClusterNestedStructList
@end

@implementation MTRUnitTestingClusterDoubleNestedStructList
- (instancetype)init
{
    if (self = [super init]) {

        _a = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRUnitTestingClusterDoubleNestedStructList alloc] init];

    other.a = self.a;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: a:%@; >", NSStringFromClass([self class]), _a];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterDoubleNestedStructList : MTRUnitTestingClusterDoubleNestedStructList
@end

@implementation MTRUnitTestingClusterTestListStructOctet
- (instancetype)init
{
    if (self = [super init]) {

        _member1 = @(0);

        _member2 = [NSData data];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRUnitTestingClusterTestListStructOctet alloc] init];

    other.member1 = self.member1;
    other.member2 = self.member2;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: member1:%@; member2:%@; >", NSStringFromClass([self class]),
                                             _member1, [_member2 base64EncodedStringWithOptions:0]];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterTestListStructOctet : MTRUnitTestingClusterTestListStructOctet
@end

@implementation MTRUnitTestingClusterTestEventEvent
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = @(0);

        _arg2 = @(0);

        _arg3 = @(0);

        _arg4 = [MTRUnitTestingClusterSimpleStruct new];

        _arg5 = [NSArray array];

        _arg6 = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRUnitTestingClusterTestEventEvent alloc] init];

    other.arg1 = self.arg1;
    other.arg2 = self.arg2;
    other.arg3 = self.arg3;
    other.arg4 = self.arg4;
    other.arg5 = self.arg5;
    other.arg6 = self.arg6;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; arg2:%@; arg3:%@; arg4:%@; arg5:%@; arg6:%@; >",
                                             NSStringFromClass([self class]), _arg1, _arg2, _arg3, _arg4, _arg5, _arg6];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterTestEventEvent : MTRUnitTestingClusterTestEventEvent
@end

@implementation MTRUnitTestingClusterTestFabricScopedEventEvent
- (instancetype)init
{
    if (self = [super init]) {

        _fabricIndex = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRUnitTestingClusterTestFabricScopedEventEvent alloc] init];

    other.fabricIndex = self.fabricIndex;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: fabricIndex:%@; >", NSStringFromClass([self class]), _fabricIndex];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterTestFabricScopedEventEvent : MTRUnitTestingClusterTestFabricScopedEventEvent
@end

NS_ASSUME_NONNULL_END
