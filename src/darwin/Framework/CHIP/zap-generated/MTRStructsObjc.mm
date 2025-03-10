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

@implementation MTRDataTypeTestGlobalStruct
- (instancetype)init
{
    if (self = [super init]) {

        _name = @"";

        _myBitmap = nil;

        _myEnum = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDataTypeTestGlobalStruct alloc] init];

    other.name = self.name;
    other.myBitmap = self.myBitmap;
    other.myEnum = self.myEnum;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: name:%@; myBitmap:%@; myEnum:%@; >", NSStringFromClass([self class]), _name, _myBitmap, _myEnum];
    return descriptionString;
}

@end

@implementation MTRDataTypeLocationDescriptorStruct
- (instancetype)init
{
    if (self = [super init]) {

        _locationName = @"";

        _floorNumber = nil;

        _areaType = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDataTypeLocationDescriptorStruct alloc] init];

    other.locationName = self.locationName;
    other.floorNumber = self.floorNumber;
    other.areaType = self.areaType;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: locationName:%@; floorNumber:%@; areaType:%@; >", NSStringFromClass([self class]), _locationName, _floorNumber, _areaType];
    return descriptionString;
}

@end

@implementation MTRDataTypeAtomicAttributeStatusStruct
- (instancetype)init
{
    if (self = [super init]) {

        _attributeID = @(0);

        _statusCode = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDataTypeAtomicAttributeStatusStruct alloc] init];

    other.attributeID = self.attributeID;
    other.statusCode = self.statusCode;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: attributeID:%@; statusCode:%@; >", NSStringFromClass([self class]), _attributeID, _statusCode];
    return descriptionString;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: deviceType:%@; revision:%@; >", NSStringFromClass([self class]), _deviceType, _revision];
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
@dynamic revision;
@end

@implementation MTRDescriptorClusterSemanticTagStruct
- (instancetype)init
{
    if (self = [super init]) {

        _mfgCode = nil;

        _namespaceID = @(0);

        _tag = @(0);

        _label = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDescriptorClusterSemanticTagStruct alloc] init];

    other.mfgCode = self.mfgCode;
    other.namespaceID = self.namespaceID;
    other.tag = self.tag;
    other.label = self.label;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: mfgCode:%@; namespaceID:%@; tag:%@; label:%@; >", NSStringFromClass([self class]), _mfgCode, _namespaceID, _tag, _label];
    return descriptionString;
}

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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: node:%@; group:%@; endpoint:%@; cluster:%@; fabricIndex:%@; >", NSStringFromClass([self class]), _node, _group, _endpoint, _cluster, _fabricIndex];
    return descriptionString;
}

@end

@implementation MTRAccessControlClusterAccessRestrictionStruct
- (instancetype)init
{
    if (self = [super init]) {

        _type = @(0);

        _id = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRAccessControlClusterAccessRestrictionStruct alloc] init];

    other.type = self.type;
    other.id = self.id;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: type:%@; id:%@; >", NSStringFromClass([self class]), _type, _id];
    return descriptionString;
}

@end

@implementation MTRAccessControlClusterCommissioningAccessRestrictionEntryStruct
- (instancetype)init
{
    if (self = [super init]) {

        _endpoint = @(0);

        _cluster = @(0);

        _restrictions = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRAccessControlClusterCommissioningAccessRestrictionEntryStruct alloc] init];

    other.endpoint = self.endpoint;
    other.cluster = self.cluster;
    other.restrictions = self.restrictions;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: endpoint:%@; cluster:%@; restrictions:%@; >", NSStringFromClass([self class]), _endpoint, _cluster, _restrictions];
    return descriptionString;
}

@end

@implementation MTRAccessControlClusterAccessRestrictionEntryStruct
- (instancetype)init
{
    if (self = [super init]) {

        _endpoint = @(0);

        _cluster = @(0);

        _restrictions = [NSArray array];

        _fabricIndex = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRAccessControlClusterAccessRestrictionEntryStruct alloc] init];

    other.endpoint = self.endpoint;
    other.cluster = self.cluster;
    other.restrictions = self.restrictions;
    other.fabricIndex = self.fabricIndex;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: endpoint:%@; cluster:%@; restrictions:%@; fabricIndex:%@; >", NSStringFromClass([self class]), _endpoint, _cluster, _restrictions, _fabricIndex];
    return descriptionString;
}

@end

@implementation MTRAccessControlClusterAccessControlTargetStruct
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
    auto other = [[MTRAccessControlClusterAccessControlTargetStruct alloc] init];

    other.cluster = self.cluster;
    other.endpoint = self.endpoint;
    other.deviceType = self.deviceType;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: cluster:%@; endpoint:%@; deviceType:%@; >", NSStringFromClass([self class]), _cluster, _endpoint, _deviceType];
    return descriptionString;
}

@end

@implementation MTRAccessControlClusterTarget : MTRAccessControlClusterAccessControlTargetStruct
@dynamic cluster;
@dynamic endpoint;
@dynamic deviceType;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: privilege:%@; authMode:%@; subjects:%@; targets:%@; fabricIndex:%@; >", NSStringFromClass([self class]), _privilege, _authMode, _subjects, _targets, _fabricIndex];
    return descriptionString;
}

@end

@implementation MTRAccessControlClusterAccessControlEntry : MTRAccessControlClusterAccessControlEntryStruct
@dynamic privilege;
@dynamic authMode;
@dynamic subjects;
@dynamic targets;
@dynamic fabricIndex;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: data:%@; fabricIndex:%@; >", NSStringFromClass([self class]), [_data base64EncodedStringWithOptions:0], _fabricIndex];
    return descriptionString;
}

@end

@implementation MTRAccessControlClusterExtensionEntry : MTRAccessControlClusterAccessControlExtensionStruct
@dynamic data;
@dynamic fabricIndex;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: adminNodeID:%@; adminPasscodeID:%@; changeType:%@; latestValue:%@; fabricIndex:%@; >", NSStringFromClass([self class]), _adminNodeID, _adminPasscodeID, _changeType, _latestValue, _fabricIndex];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: adminNodeID:%@; adminPasscodeID:%@; changeType:%@; latestValue:%@; fabricIndex:%@; >", NSStringFromClass([self class]), _adminNodeID, _adminPasscodeID, _changeType, _latestValue, _fabricIndex];
    return descriptionString;
}

@end

@implementation MTRAccessControlClusterFabricRestrictionReviewUpdateEvent
- (instancetype)init
{
    if (self = [super init]) {

        _token = @(0);

        _instruction = nil;

        _arlRequestFlowUrl = nil;

        _fabricIndex = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRAccessControlClusterFabricRestrictionReviewUpdateEvent alloc] init];

    other.token = self.token;
    other.instruction = self.instruction;
    other.arlRequestFlowUrl = self.arlRequestFlowUrl;
    other.fabricIndex = self.fabricIndex;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: token:%@; instruction:%@; arlRequestFlowUrl:%@; fabricIndex:%@; >", NSStringFromClass([self class]), _token, _instruction, _arlRequestFlowUrl, _fabricIndex];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: actionID:%@; name:%@; type:%@; endpointListID:%@; supportedCommands:%@; state:%@; >", NSStringFromClass([self class]), _actionID, _name, _type, _endpointListID, _supportedCommands, _state];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: endpointListID:%@; name:%@; type:%@; endpoints:%@; >", NSStringFromClass([self class]), _endpointListID, _name, _type, _endpoints];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: actionID:%@; invokeID:%@; newState:%@; >", NSStringFromClass([self class]), _actionID, _invokeID, _newState];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: actionID:%@; invokeID:%@; newState:%@; error:%@; >", NSStringFromClass([self class]), _actionID, _invokeID, _newState, _error];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: caseSessionsPerFabric:%@; subscriptionsPerFabric:%@; >", NSStringFromClass([self class]), _caseSessionsPerFabric, _subscriptionsPerFabric];
    return descriptionString;
}

@end

@implementation MTRBasicClusterCapabilityMinimaStruct : MTRBasicInformationClusterCapabilityMinimaStruct
@dynamic caseSessionsPerFabric;
@dynamic subscriptionsPerFabric;
@end

@implementation MTRBasicInformationClusterProductAppearanceStruct
- (instancetype)init
{
    if (self = [super init]) {

        _finish = @(0);

        _primaryColor = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRBasicInformationClusterProductAppearanceStruct alloc] init];

    other.finish = self.finish;
    other.primaryColor = self.primaryColor;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: finish:%@; primaryColor:%@; >", NSStringFromClass([self class]), _finish, _primaryColor];
    return descriptionString;
}

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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: softwareVersion:%@; >", NSStringFromClass([self class]), _softwareVersion];
    return descriptionString;
}

@end

@implementation MTRBasicClusterStartUpEvent : MTRBasicInformationClusterStartUpEvent
@dynamic softwareVersion;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: fabricIndex:%@; >", NSStringFromClass([self class]), _fabricIndex];
    return descriptionString;
}

@end

@implementation MTRBasicClusterLeaveEvent : MTRBasicInformationClusterLeaveEvent
@dynamic fabricIndex;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: reachableNewValue:%@; >", NSStringFromClass([self class]), _reachableNewValue];
    return descriptionString;
}

@end

@implementation MTRBasicClusterReachableChangedEvent : MTRBasicInformationClusterReachableChangedEvent
@dynamic reachableNewValue;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: providerNodeID:%@; endpoint:%@; fabricIndex:%@; >", NSStringFromClass([self class]), _providerNodeID, _endpoint, _fabricIndex];
    return descriptionString;
}

@end

@implementation MTROtaSoftwareUpdateRequestorClusterProviderLocation : MTROTASoftwareUpdateRequestorClusterProviderLocation
@dynamic providerNodeID;
@dynamic endpoint;
@dynamic fabricIndex;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: previousState:%@; newState:%@; reason:%@; targetSoftwareVersion:%@; >", NSStringFromClass([self class]), _previousState, _newState, _reason, _targetSoftwareVersion];
    return descriptionString;
}

@end

@implementation MTROtaSoftwareUpdateRequestorClusterStateTransitionEvent : MTROTASoftwareUpdateRequestorClusterStateTransitionEvent
@dynamic previousState;
@dynamic newState;
@dynamic reason;
@dynamic targetSoftwareVersion;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: softwareVersion:%@; productID:%@; >", NSStringFromClass([self class]), _softwareVersion, _productID];
    return descriptionString;
}

@end

@implementation MTROtaSoftwareUpdateRequestorClusterVersionAppliedEvent : MTROTASoftwareUpdateRequestorClusterVersionAppliedEvent
@dynamic softwareVersion;
@dynamic productID;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: softwareVersion:%@; bytesDownloaded:%@; progressPercent:%@; platformCode:%@; >", NSStringFromClass([self class]), _softwareVersion, _bytesDownloaded, _progressPercent, _platformCode];
    return descriptionString;
}

@end

@implementation MTROtaSoftwareUpdateRequestorClusterDownloadErrorEvent : MTROTASoftwareUpdateRequestorClusterDownloadErrorEvent
@dynamic softwareVersion;
@dynamic bytesDownloaded;
@dynamic progressPercent;
@dynamic platformCode;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: current:%@; previous:%@; >", NSStringFromClass([self class]), _current, _previous];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: current:%@; previous:%@; >", NSStringFromClass([self class]), _current, _previous];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: current:%@; previous:%@; >", NSStringFromClass([self class]), _current, _previous];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: current:%@; previous:%@; >", NSStringFromClass([self class]), _current, _previous];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: current:%@; previous:%@; >", NSStringFromClass([self class]), _current, _previous];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: current:%@; previous:%@; >", NSStringFromClass([self class]), _current, _previous];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: failSafeExpiryLengthSeconds:%@; maxCumulativeFailsafeSeconds:%@; >", NSStringFromClass([self class]), _failSafeExpiryLengthSeconds, _maxCumulativeFailsafeSeconds];
    return descriptionString;
}

@end

@implementation MTRNetworkCommissioningClusterNetworkInfoStruct
- (instancetype)init
{
    if (self = [super init]) {

        _networkID = [NSData data];

        _connected = @(0);

        _networkIdentifier = nil;

        _clientIdentifier = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRNetworkCommissioningClusterNetworkInfoStruct alloc] init];

    other.networkID = self.networkID;
    other.connected = self.connected;
    other.networkIdentifier = self.networkIdentifier;
    other.clientIdentifier = self.clientIdentifier;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: networkID:%@; connected:%@; networkIdentifier:%@; clientIdentifier:%@; >", NSStringFromClass([self class]), [_networkID base64EncodedStringWithOptions:0], _connected, [_networkIdentifier base64EncodedStringWithOptions:0], [_clientIdentifier base64EncodedStringWithOptions:0]];
    return descriptionString;
}

@end

@implementation MTRNetworkCommissioningClusterNetworkInfo : MTRNetworkCommissioningClusterNetworkInfoStruct
@dynamic networkID;
@dynamic connected;
@end

@implementation MTRNetworkCommissioningClusterThreadInterfaceScanResultStruct
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
    auto other = [[MTRNetworkCommissioningClusterThreadInterfaceScanResultStruct alloc] init];

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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: panId:%@; extendedPanId:%@; networkName:%@; channel:%@; version:%@; extendedAddress:%@; rssi:%@; lqi:%@; >", NSStringFromClass([self class]), _panId, _extendedPanId, _networkName, _channel, _version, [_extendedAddress base64EncodedStringWithOptions:0], _rssi, _lqi];
    return descriptionString;
}

@end

@implementation MTRNetworkCommissioningClusterThreadInterfaceScanResult : MTRNetworkCommissioningClusterThreadInterfaceScanResultStruct
@dynamic panId;
@dynamic extendedPanId;
@dynamic networkName;
@dynamic channel;
@dynamic version;
@dynamic extendedAddress;
@dynamic rssi;
@dynamic lqi;
@end

@implementation MTRNetworkCommissioningClusterWiFiInterfaceScanResultStruct
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
    auto other = [[MTRNetworkCommissioningClusterWiFiInterfaceScanResultStruct alloc] init];

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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: security:%@; ssid:%@; bssid:%@; channel:%@; wiFiBand:%@; rssi:%@; >", NSStringFromClass([self class]), _security, [_ssid base64EncodedStringWithOptions:0], [_bssid base64EncodedStringWithOptions:0], _channel, _wiFiBand, _rssi];
    return descriptionString;
}

@end

@implementation MTRNetworkCommissioningClusterWiFiInterfaceScanResult : MTRNetworkCommissioningClusterWiFiInterfaceScanResultStruct
@dynamic security;
@dynamic ssid;
@dynamic bssid;
@dynamic channel;
@dynamic wiFiBand;
@dynamic rssi;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: name:%@; isOperational:%@; offPremiseServicesReachableIPv4:%@; offPremiseServicesReachableIPv6:%@; hardwareAddress:%@; iPv4Addresses:%@; iPv6Addresses:%@; type:%@; >", NSStringFromClass([self class]), _name, _isOperational, _offPremiseServicesReachableIPv4, _offPremiseServicesReachableIPv6, [_hardwareAddress base64EncodedStringWithOptions:0], _iPv4Addresses, _iPv6Addresses, _type];
    return descriptionString;
}

@end

@implementation MTRGeneralDiagnosticsClusterNetworkInterfaceType : MTRGeneralDiagnosticsClusterNetworkInterface
@dynamic name;
@dynamic isOperational;
@dynamic offPremiseServicesReachableIPv4;
@dynamic offPremiseServicesReachableIPv6;
@dynamic hardwareAddress;
@dynamic iPv4Addresses;
@dynamic iPv6Addresses;
@dynamic type;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: current:%@; previous:%@; >", NSStringFromClass([self class]), _current, _previous];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: current:%@; previous:%@; >", NSStringFromClass([self class]), _current, _previous];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: current:%@; previous:%@; >", NSStringFromClass([self class]), _current, _previous];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: bootReason:%@; >", NSStringFromClass([self class]), _bootReason];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: id:%@; name:%@; stackFreeCurrent:%@; stackFreeMinimum:%@; stackSize:%@; >", NSStringFromClass([self class]), _id, _name, _stackFreeCurrent, _stackFreeMinimum, _stackSize];
    return descriptionString;
}

@end

@implementation MTRSoftwareDiagnosticsClusterThreadMetrics : MTRSoftwareDiagnosticsClusterThreadMetricsStruct
@dynamic id;
@dynamic name;
@dynamic stackFreeCurrent;
@dynamic stackFreeMinimum;
@dynamic stackSize;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: id:%@; name:%@; faultRecording:%@; >", NSStringFromClass([self class]), _id, _name, [_faultRecording base64EncodedStringWithOptions:0]];
    return descriptionString;
}

@end

@implementation MTRThreadNetworkDiagnosticsClusterNeighborTableStruct
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
    auto other = [[MTRThreadNetworkDiagnosticsClusterNeighborTableStruct alloc] init];

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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: extAddress:%@; age:%@; rloc16:%@; linkFrameCounter:%@; mleFrameCounter:%@; lqi:%@; averageRssi:%@; lastRssi:%@; frameErrorRate:%@; messageErrorRate:%@; rxOnWhenIdle:%@; fullThreadDevice:%@; fullNetworkData:%@; isChild:%@; >", NSStringFromClass([self class]), _extAddress, _age, _rloc16, _linkFrameCounter, _mleFrameCounter, _lqi, _averageRssi, _lastRssi, _frameErrorRate, _messageErrorRate, _rxOnWhenIdle, _fullThreadDevice, _fullNetworkData, _isChild];
    return descriptionString;
}

@end

@implementation MTRThreadNetworkDiagnosticsClusterNeighborTable : MTRThreadNetworkDiagnosticsClusterNeighborTableStruct
@dynamic extAddress;
@dynamic age;
@dynamic rloc16;
@dynamic linkFrameCounter;
@dynamic mleFrameCounter;
@dynamic lqi;
@dynamic averageRssi;
@dynamic lastRssi;
@dynamic frameErrorRate;
@dynamic messageErrorRate;
@dynamic rxOnWhenIdle;
@dynamic fullThreadDevice;
@dynamic fullNetworkData;
@dynamic isChild;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: activeTimestampPresent:%@; pendingTimestampPresent:%@; masterKeyPresent:%@; networkNamePresent:%@; extendedPanIdPresent:%@; meshLocalPrefixPresent:%@; delayPresent:%@; panIdPresent:%@; channelPresent:%@; pskcPresent:%@; securityPolicyPresent:%@; channelMaskPresent:%@; >", NSStringFromClass([self class]), _activeTimestampPresent, _pendingTimestampPresent, _masterKeyPresent, _networkNamePresent, _extendedPanIdPresent, _meshLocalPrefixPresent, _delayPresent, _panIdPresent, _channelPresent, _pskcPresent, _securityPolicyPresent, _channelMaskPresent];
    return descriptionString;
}

@end

@implementation MTRThreadNetworkDiagnosticsClusterRouteTableStruct
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
    auto other = [[MTRThreadNetworkDiagnosticsClusterRouteTableStruct alloc] init];

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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: extAddress:%@; rloc16:%@; routerId:%@; nextHop:%@; pathCost:%@; lqiIn:%@; lqiOut:%@; age:%@; allocated:%@; linkEstablished:%@; >", NSStringFromClass([self class]), _extAddress, _rloc16, _routerId, _nextHop, _pathCost, _lqiIn, _lqiOut, _age, _allocated, _linkEstablished];
    return descriptionString;
}

@end

@implementation MTRThreadNetworkDiagnosticsClusterRouteTable : MTRThreadNetworkDiagnosticsClusterRouteTableStruct
@dynamic extAddress;
@dynamic rloc16;
@dynamic routerId;
@dynamic nextHop;
@dynamic pathCost;
@dynamic lqiIn;
@dynamic lqiOut;
@dynamic age;
@dynamic allocated;
@dynamic linkEstablished;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: rotationTime:%@; flags:%@; >", NSStringFromClass([self class]), _rotationTime, _flags];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: connectionStatus:%@; >", NSStringFromClass([self class]), _connectionStatus];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: current:%@; previous:%@; >", NSStringFromClass([self class]), _current, _previous];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: reasonCode:%@; >", NSStringFromClass([self class]), _reasonCode];
    return descriptionString;
}

@end

@implementation MTRWiFiNetworkDiagnosticsClusterAssociationFailureEvent
- (instancetype)init
{
    if (self = [super init]) {

        _associationFailureCause = @(0);

        _status = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRWiFiNetworkDiagnosticsClusterAssociationFailureEvent alloc] init];

    other.associationFailureCause = self.associationFailureCause;
    other.status = self.status;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: associationFailureCause:%@; status:%@; >", NSStringFromClass([self class]), _associationFailureCause, _status];
    return descriptionString;
}

- (void)setAssociationFailure:(NSNumber * _Nonnull)associationFailure
{
    self.associationFailureCause = associationFailure;
}

- (NSNumber * _Nonnull)associationFailure
{
    return self.associationFailureCause;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: connectionStatus:%@; >", NSStringFromClass([self class]), _connectionStatus];
    return descriptionString;
}

@end

@implementation MTRTimeSynchronizationClusterDSTOffsetStruct
- (instancetype)init
{
    if (self = [super init]) {

        _offset = @(0);

        _validStarting = @(0);

        _validUntil = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRTimeSynchronizationClusterDSTOffsetStruct alloc] init];

    other.offset = self.offset;
    other.validStarting = self.validStarting;
    other.validUntil = self.validUntil;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: offset:%@; validStarting:%@; validUntil:%@; >", NSStringFromClass([self class]), _offset, _validStarting, _validUntil];
    return descriptionString;
}

@end

@implementation MTRTimeSynchronizationClusterDstOffsetType : MTRTimeSynchronizationClusterDSTOffsetStruct
@dynamic offset;
@dynamic validStarting;
@dynamic validUntil;
@end

@implementation MTRTimeSynchronizationClusterFabricScopedTrustedTimeSourceStruct
- (instancetype)init
{
    if (self = [super init]) {

        _nodeID = @(0);

        _endpoint = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRTimeSynchronizationClusterFabricScopedTrustedTimeSourceStruct alloc] init];

    other.nodeID = self.nodeID;
    other.endpoint = self.endpoint;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: nodeID:%@; endpoint:%@; >", NSStringFromClass([self class]), _nodeID, _endpoint];
    return descriptionString;
}

@end

@implementation MTRTimeSynchronizationClusterTimeZoneStruct
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
    auto other = [[MTRTimeSynchronizationClusterTimeZoneStruct alloc] init];

    other.offset = self.offset;
    other.validAt = self.validAt;
    other.name = self.name;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: offset:%@; validAt:%@; name:%@; >", NSStringFromClass([self class]), _offset, _validAt, _name];
    return descriptionString;
}

@end

@implementation MTRTimeSynchronizationClusterTimeZoneType : MTRTimeSynchronizationClusterTimeZoneStruct
@dynamic offset;
@dynamic validAt;
@dynamic name;
@end

@implementation MTRTimeSynchronizationClusterTrustedTimeSourceStruct
- (instancetype)init
{
    if (self = [super init]) {

        _fabricIndex = @(0);

        _nodeID = @(0);

        _endpoint = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRTimeSynchronizationClusterTrustedTimeSourceStruct alloc] init];

    other.fabricIndex = self.fabricIndex;
    other.nodeID = self.nodeID;
    other.endpoint = self.endpoint;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: fabricIndex:%@; nodeID:%@; endpoint:%@; >", NSStringFromClass([self class]), _fabricIndex, _nodeID, _endpoint];
    return descriptionString;
}

@end

@implementation MTRTimeSynchronizationClusterDSTTableEmptyEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRTimeSynchronizationClusterDSTTableEmptyEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRTimeSynchronizationClusterDSTStatusEvent
- (instancetype)init
{
    if (self = [super init]) {

        _dstOffsetActive = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRTimeSynchronizationClusterDSTStatusEvent alloc] init];

    other.dstOffsetActive = self.dstOffsetActive;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: dstOffsetActive:%@; >", NSStringFromClass([self class]), _dstOffsetActive];
    return descriptionString;
}

@end

@implementation MTRTimeSynchronizationClusterTimeZoneStatusEvent
- (instancetype)init
{
    if (self = [super init]) {

        _offset = @(0);

        _name = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRTimeSynchronizationClusterTimeZoneStatusEvent alloc] init];

    other.offset = self.offset;
    other.name = self.name;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: offset:%@; name:%@; >", NSStringFromClass([self class]), _offset, _name];
    return descriptionString;
}

@end

@implementation MTRTimeSynchronizationClusterTimeFailureEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRTimeSynchronizationClusterTimeFailureEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRTimeSynchronizationClusterMissingTrustedTimeSourceEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRTimeSynchronizationClusterMissingTrustedTimeSourceEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRBridgedDeviceBasicInformationClusterProductAppearanceStruct
- (instancetype)init
{
    if (self = [super init]) {

        _finish = @(0);

        _primaryColor = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRBridgedDeviceBasicInformationClusterProductAppearanceStruct alloc] init];

    other.finish = self.finish;
    other.primaryColor = self.primaryColor;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: finish:%@; primaryColor:%@; >", NSStringFromClass([self class]), _finish, _primaryColor];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: softwareVersion:%@; >", NSStringFromClass([self class]), _softwareVersion];
    return descriptionString;
}

@end

@implementation MTRBridgedDeviceBasicClusterStartUpEvent : MTRBridgedDeviceBasicInformationClusterStartUpEvent
@dynamic softwareVersion;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: reachableNewValue:%@; >", NSStringFromClass([self class]), _reachableNewValue];
    return descriptionString;
}

@end

@implementation MTRBridgedDeviceBasicClusterReachableChangedEvent : MTRBridgedDeviceBasicInformationClusterReachableChangedEvent
@dynamic reachableNewValue;
@end

@implementation MTRBridgedDeviceBasicInformationClusterActiveChangedEvent
- (instancetype)init
{
    if (self = [super init]) {

        _promisedActiveDuration = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRBridgedDeviceBasicInformationClusterActiveChangedEvent alloc] init];

    other.promisedActiveDuration = self.promisedActiveDuration;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: promisedActiveDuration:%@; >", NSStringFromClass([self class]), _promisedActiveDuration];
    return descriptionString;
}

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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: newPosition:%@; >", NSStringFromClass([self class]), _newPosition];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: newPosition:%@; >", NSStringFromClass([self class]), _newPosition];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: newPosition:%@; >", NSStringFromClass([self class]), _newPosition];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: previousPosition:%@; >", NSStringFromClass([self class]), _previousPosition];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: previousPosition:%@; >", NSStringFromClass([self class]), _previousPosition];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: newPosition:%@; currentNumberOfPressesCounted:%@; >", NSStringFromClass([self class]), _newPosition, _currentNumberOfPressesCounted];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: previousPosition:%@; totalNumberOfPressesCounted:%@; >", NSStringFromClass([self class]), _previousPosition, _totalNumberOfPressesCounted];
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

        _vidVerificationStatement = nil;

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
    other.vidVerificationStatement = self.vidVerificationStatement;
    other.fabricIndex = self.fabricIndex;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: rootPublicKey:%@; vendorID:%@; fabricID:%@; nodeID:%@; label:%@; vidVerificationStatement:%@; fabricIndex:%@; >", NSStringFromClass([self class]), [_rootPublicKey base64EncodedStringWithOptions:0], _vendorID, _fabricID, _nodeID, _label, [_vidVerificationStatement base64EncodedStringWithOptions:0], _fabricIndex];
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
@dynamic rootPublicKey;
@dynamic label;
@dynamic fabricIndex;
@end

@implementation MTROperationalCredentialsClusterNOCStruct
- (instancetype)init
{
    if (self = [super init]) {

        _noc = [NSData data];

        _icac = nil;

        _vvsc = nil;

        _fabricIndex = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTROperationalCredentialsClusterNOCStruct alloc] init];

    other.noc = self.noc;
    other.icac = self.icac;
    other.vvsc = self.vvsc;
    other.fabricIndex = self.fabricIndex;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: noc:%@; icac:%@; vvsc:%@; fabricIndex:%@; >", NSStringFromClass([self class]), [_noc base64EncodedStringWithOptions:0], [_icac base64EncodedStringWithOptions:0], [_vvsc base64EncodedStringWithOptions:0], _fabricIndex];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: groupId:%@; endpoints:%@; groupName:%@; fabricIndex:%@; >", NSStringFromClass([self class]), _groupId, _endpoints, _groupName, _fabricIndex];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: groupId:%@; groupKeySetID:%@; fabricIndex:%@; >", NSStringFromClass([self class]), _groupId, _groupKeySetID, _fabricIndex];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: groupKeySetID:%@; groupKeySecurityPolicy:%@; epochKey0:%@; epochStartTime0:%@; epochKey1:%@; epochStartTime1:%@; epochKey2:%@; epochStartTime2:%@; >", NSStringFromClass([self class]), _groupKeySetID, _groupKeySecurityPolicy, [_epochKey0 base64EncodedStringWithOptions:0], _epochStartTime0, [_epochKey1 base64EncodedStringWithOptions:0], _epochStartTime1, [_epochKey2 base64EncodedStringWithOptions:0], _epochStartTime2];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: label:%@; value:%@; >", NSStringFromClass([self class]), _label, _value];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: label:%@; value:%@; >", NSStringFromClass([self class]), _label, _value];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: stateValue:%@; >", NSStringFromClass([self class]), _stateValue];
    return descriptionString;
}

@end

@implementation MTRICDManagementClusterMonitoringRegistrationStruct
- (instancetype)init
{
    if (self = [super init]) {

        _checkInNodeID = @(0);

        _monitoredSubject = @(0);

        _clientType = @(0);

        _fabricIndex = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRICDManagementClusterMonitoringRegistrationStruct alloc] init];

    other.checkInNodeID = self.checkInNodeID;
    other.monitoredSubject = self.monitoredSubject;
    other.clientType = self.clientType;
    other.fabricIndex = self.fabricIndex;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: checkInNodeID:%@; monitoredSubject:%@; clientType:%@; fabricIndex:%@; >", NSStringFromClass([self class]), _checkInNodeID, _monitoredSubject, _clientType, _fabricIndex];
    return descriptionString;
}

@end

@implementation MTROvenCavityOperationalStateClusterErrorStateStruct
- (instancetype)init
{
    if (self = [super init]) {

        _errorStateID = @(0);

        _errorStateLabel = nil;

        _errorStateDetails = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTROvenCavityOperationalStateClusterErrorStateStruct alloc] init];

    other.errorStateID = self.errorStateID;
    other.errorStateLabel = self.errorStateLabel;
    other.errorStateDetails = self.errorStateDetails;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: errorStateID:%@; errorStateLabel:%@; errorStateDetails:%@; >", NSStringFromClass([self class]), _errorStateID, _errorStateLabel, _errorStateDetails];
    return descriptionString;
}

@end

@implementation MTROvenCavityOperationalStateClusterOperationalStateStruct
- (instancetype)init
{
    if (self = [super init]) {

        _operationalStateID = @(0);

        _operationalStateLabel = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTROvenCavityOperationalStateClusterOperationalStateStruct alloc] init];

    other.operationalStateID = self.operationalStateID;
    other.operationalStateLabel = self.operationalStateLabel;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: operationalStateID:%@; operationalStateLabel:%@; >", NSStringFromClass([self class]), _operationalStateID, _operationalStateLabel];
    return descriptionString;
}

@end

@implementation MTROvenCavityOperationalStateClusterOperationalErrorEvent
- (instancetype)init
{
    if (self = [super init]) {

        _errorState = [MTROvenCavityOperationalStateClusterErrorStateStruct new];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTROvenCavityOperationalStateClusterOperationalErrorEvent alloc] init];

    other.errorState = self.errorState;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: errorState:%@; >", NSStringFromClass([self class]), _errorState];
    return descriptionString;
}

@end

@implementation MTROvenCavityOperationalStateClusterOperationCompletionEvent
- (instancetype)init
{
    if (self = [super init]) {

        _completionErrorCode = @(0);

        _totalOperationalTime = nil;

        _pausedTime = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTROvenCavityOperationalStateClusterOperationCompletionEvent alloc] init];

    other.completionErrorCode = self.completionErrorCode;
    other.totalOperationalTime = self.totalOperationalTime;
    other.pausedTime = self.pausedTime;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: completionErrorCode:%@; totalOperationalTime:%@; pausedTime:%@; >", NSStringFromClass([self class]), _completionErrorCode, _totalOperationalTime, _pausedTime];
    return descriptionString;
}

@end

@implementation MTROvenModeClusterModeTagStruct
- (instancetype)init
{
    if (self = [super init]) {

        _mfgCode = nil;

        _value = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTROvenModeClusterModeTagStruct alloc] init];

    other.mfgCode = self.mfgCode;
    other.value = self.value;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: mfgCode:%@; value:%@; >", NSStringFromClass([self class]), _mfgCode, _value];
    return descriptionString;
}

@end

@implementation MTROvenModeClusterModeOptionStruct
- (instancetype)init
{
    if (self = [super init]) {

        _label = @"";

        _mode = @(0);

        _modeTags = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTROvenModeClusterModeOptionStruct alloc] init];

    other.label = self.label;
    other.mode = self.mode;
    other.modeTags = self.modeTags;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: label:%@; mode:%@; modeTags:%@; >", NSStringFromClass([self class]), _label, _mode, _modeTags];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: mfgCode:%@; value:%@; >", NSStringFromClass([self class]), _mfgCode, _value];
    return descriptionString;
}

@end

@implementation MTRModeSelectClusterSemanticTag : MTRModeSelectClusterSemanticTagStruct
@dynamic mfgCode;
@dynamic value;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: label:%@; mode:%@; semanticTags:%@; >", NSStringFromClass([self class]), _label, _mode, _semanticTags];
    return descriptionString;
}

@end

@implementation MTRLaundryWasherModeClusterModeTagStruct
- (instancetype)init
{
    if (self = [super init]) {

        _mfgCode = nil;

        _value = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRLaundryWasherModeClusterModeTagStruct alloc] init];

    other.mfgCode = self.mfgCode;
    other.value = self.value;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: mfgCode:%@; value:%@; >", NSStringFromClass([self class]), _mfgCode, _value];
    return descriptionString;
}

@end

@implementation MTRLaundryWasherModeClusterModeOptionStruct
- (instancetype)init
{
    if (self = [super init]) {

        _label = @"";

        _mode = @(0);

        _modeTags = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRLaundryWasherModeClusterModeOptionStruct alloc] init];

    other.label = self.label;
    other.mode = self.mode;
    other.modeTags = self.modeTags;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: label:%@; mode:%@; modeTags:%@; >", NSStringFromClass([self class]), _label, _mode, _modeTags];
    return descriptionString;
}

@end

@implementation MTRRefrigeratorAndTemperatureControlledCabinetModeClusterModeTagStruct
- (instancetype)init
{
    if (self = [super init]) {

        _mfgCode = nil;

        _value = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRRefrigeratorAndTemperatureControlledCabinetModeClusterModeTagStruct alloc] init];

    other.mfgCode = self.mfgCode;
    other.value = self.value;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: mfgCode:%@; value:%@; >", NSStringFromClass([self class]), _mfgCode, _value];
    return descriptionString;
}

@end

@implementation MTRRefrigeratorAndTemperatureControlledCabinetModeClusterModeOptionStruct
- (instancetype)init
{
    if (self = [super init]) {

        _label = @"";

        _mode = @(0);

        _modeTags = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRRefrigeratorAndTemperatureControlledCabinetModeClusterModeOptionStruct alloc] init];

    other.label = self.label;
    other.mode = self.mode;
    other.modeTags = self.modeTags;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: label:%@; mode:%@; modeTags:%@; >", NSStringFromClass([self class]), _label, _mode, _modeTags];
    return descriptionString;
}

@end

@implementation MTRRVCRunModeClusterModeTagStruct
- (instancetype)init
{
    if (self = [super init]) {

        _mfgCode = nil;

        _value = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRRVCRunModeClusterModeTagStruct alloc] init];

    other.mfgCode = self.mfgCode;
    other.value = self.value;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: mfgCode:%@; value:%@; >", NSStringFromClass([self class]), _mfgCode, _value];
    return descriptionString;
}

@end

@implementation MTRRVCRunModeClusterModeOptionStruct
- (instancetype)init
{
    if (self = [super init]) {

        _label = @"";

        _mode = @(0);

        _modeTags = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRRVCRunModeClusterModeOptionStruct alloc] init];

    other.label = self.label;
    other.mode = self.mode;
    other.modeTags = self.modeTags;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: label:%@; mode:%@; modeTags:%@; >", NSStringFromClass([self class]), _label, _mode, _modeTags];
    return descriptionString;
}

@end

@implementation MTRRVCCleanModeClusterModeTagStruct
- (instancetype)init
{
    if (self = [super init]) {

        _mfgCode = nil;

        _value = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRRVCCleanModeClusterModeTagStruct alloc] init];

    other.mfgCode = self.mfgCode;
    other.value = self.value;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: mfgCode:%@; value:%@; >", NSStringFromClass([self class]), _mfgCode, _value];
    return descriptionString;
}

@end

@implementation MTRRVCCleanModeClusterModeOptionStruct
- (instancetype)init
{
    if (self = [super init]) {

        _label = @"";

        _mode = @(0);

        _modeTags = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRRVCCleanModeClusterModeOptionStruct alloc] init];

    other.label = self.label;
    other.mode = self.mode;
    other.modeTags = self.modeTags;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: label:%@; mode:%@; modeTags:%@; >", NSStringFromClass([self class]), _label, _mode, _modeTags];
    return descriptionString;
}

@end

@implementation MTRRefrigeratorAlarmClusterNotifyEvent
- (instancetype)init
{
    if (self = [super init]) {

        _active = @(0);

        _inactive = @(0);

        _state = @(0);

        _mask = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRRefrigeratorAlarmClusterNotifyEvent alloc] init];

    other.active = self.active;
    other.inactive = self.inactive;
    other.state = self.state;
    other.mask = self.mask;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: active:%@; inactive:%@; state:%@; mask:%@; >", NSStringFromClass([self class]), _active, _inactive, _state, _mask];
    return descriptionString;
}

@end

@implementation MTRDishwasherModeClusterModeTagStruct
- (instancetype)init
{
    if (self = [super init]) {

        _mfgCode = nil;

        _value = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDishwasherModeClusterModeTagStruct alloc] init];

    other.mfgCode = self.mfgCode;
    other.value = self.value;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: mfgCode:%@; value:%@; >", NSStringFromClass([self class]), _mfgCode, _value];
    return descriptionString;
}

@end

@implementation MTRDishwasherModeClusterModeOptionStruct
- (instancetype)init
{
    if (self = [super init]) {

        _label = @"";

        _mode = @(0);

        _modeTags = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDishwasherModeClusterModeOptionStruct alloc] init];

    other.label = self.label;
    other.mode = self.mode;
    other.modeTags = self.modeTags;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: label:%@; mode:%@; modeTags:%@; >", NSStringFromClass([self class]), _label, _mode, _modeTags];
    return descriptionString;
}

@end

@implementation MTRSmokeCOAlarmClusterSmokeAlarmEvent
- (instancetype)init
{
    if (self = [super init]) {

        _alarmSeverityLevel = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRSmokeCOAlarmClusterSmokeAlarmEvent alloc] init];

    other.alarmSeverityLevel = self.alarmSeverityLevel;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: alarmSeverityLevel:%@; >", NSStringFromClass([self class]), _alarmSeverityLevel];
    return descriptionString;
}

@end

@implementation MTRSmokeCOAlarmClusterCOAlarmEvent
- (instancetype)init
{
    if (self = [super init]) {

        _alarmSeverityLevel = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRSmokeCOAlarmClusterCOAlarmEvent alloc] init];

    other.alarmSeverityLevel = self.alarmSeverityLevel;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: alarmSeverityLevel:%@; >", NSStringFromClass([self class]), _alarmSeverityLevel];
    return descriptionString;
}

@end

@implementation MTRSmokeCOAlarmClusterLowBatteryEvent
- (instancetype)init
{
    if (self = [super init]) {

        _alarmSeverityLevel = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRSmokeCOAlarmClusterLowBatteryEvent alloc] init];

    other.alarmSeverityLevel = self.alarmSeverityLevel;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: alarmSeverityLevel:%@; >", NSStringFromClass([self class]), _alarmSeverityLevel];
    return descriptionString;
}

@end

@implementation MTRSmokeCOAlarmClusterHardwareFaultEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRSmokeCOAlarmClusterHardwareFaultEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRSmokeCOAlarmClusterEndOfServiceEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRSmokeCOAlarmClusterEndOfServiceEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRSmokeCOAlarmClusterSelfTestCompleteEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRSmokeCOAlarmClusterSelfTestCompleteEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRSmokeCOAlarmClusterAlarmMutedEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRSmokeCOAlarmClusterAlarmMutedEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRSmokeCOAlarmClusterMuteEndedEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRSmokeCOAlarmClusterMuteEndedEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRSmokeCOAlarmClusterInterconnectSmokeAlarmEvent
- (instancetype)init
{
    if (self = [super init]) {

        _alarmSeverityLevel = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRSmokeCOAlarmClusterInterconnectSmokeAlarmEvent alloc] init];

    other.alarmSeverityLevel = self.alarmSeverityLevel;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: alarmSeverityLevel:%@; >", NSStringFromClass([self class]), _alarmSeverityLevel];
    return descriptionString;
}

@end

@implementation MTRSmokeCOAlarmClusterInterconnectCOAlarmEvent
- (instancetype)init
{
    if (self = [super init]) {

        _alarmSeverityLevel = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRSmokeCOAlarmClusterInterconnectCOAlarmEvent alloc] init];

    other.alarmSeverityLevel = self.alarmSeverityLevel;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: alarmSeverityLevel:%@; >", NSStringFromClass([self class]), _alarmSeverityLevel];
    return descriptionString;
}

@end

@implementation MTRSmokeCOAlarmClusterAllClearEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRSmokeCOAlarmClusterAllClearEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRDishwasherAlarmClusterNotifyEvent
- (instancetype)init
{
    if (self = [super init]) {

        _active = @(0);

        _inactive = @(0);

        _state = @(0);

        _mask = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDishwasherAlarmClusterNotifyEvent alloc] init];

    other.active = self.active;
    other.inactive = self.inactive;
    other.state = self.state;
    other.mask = self.mask;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: active:%@; inactive:%@; state:%@; mask:%@; >", NSStringFromClass([self class]), _active, _inactive, _state, _mask];
    return descriptionString;
}

@end

@implementation MTRMicrowaveOvenModeClusterModeTagStruct
- (instancetype)init
{
    if (self = [super init]) {

        _mfgCode = nil;

        _value = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRMicrowaveOvenModeClusterModeTagStruct alloc] init];

    other.mfgCode = self.mfgCode;
    other.value = self.value;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: mfgCode:%@; value:%@; >", NSStringFromClass([self class]), _mfgCode, _value];
    return descriptionString;
}

@end

@implementation MTRMicrowaveOvenModeClusterModeOptionStruct
- (instancetype)init
{
    if (self = [super init]) {

        _label = @"";

        _mode = @(0);

        _modeTags = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRMicrowaveOvenModeClusterModeOptionStruct alloc] init];

    other.label = self.label;
    other.mode = self.mode;
    other.modeTags = self.modeTags;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: label:%@; mode:%@; modeTags:%@; >", NSStringFromClass([self class]), _label, _mode, _modeTags];
    return descriptionString;
}

@end

@implementation MTROperationalStateClusterErrorStateStruct
- (instancetype)init
{
    if (self = [super init]) {

        _errorStateID = @(0);

        _errorStateLabel = nil;

        _errorStateDetails = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTROperationalStateClusterErrorStateStruct alloc] init];

    other.errorStateID = self.errorStateID;
    other.errorStateLabel = self.errorStateLabel;
    other.errorStateDetails = self.errorStateDetails;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: errorStateID:%@; errorStateLabel:%@; errorStateDetails:%@; >", NSStringFromClass([self class]), _errorStateID, _errorStateLabel, _errorStateDetails];
    return descriptionString;
}

@end

@implementation MTROperationalStateClusterOperationalStateStruct
- (instancetype)init
{
    if (self = [super init]) {

        _operationalStateID = @(0);

        _operationalStateLabel = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTROperationalStateClusterOperationalStateStruct alloc] init];

    other.operationalStateID = self.operationalStateID;
    other.operationalStateLabel = self.operationalStateLabel;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: operationalStateID:%@; operationalStateLabel:%@; >", NSStringFromClass([self class]), _operationalStateID, _operationalStateLabel];
    return descriptionString;
}

@end

@implementation MTROperationalStateClusterOperationalErrorEvent
- (instancetype)init
{
    if (self = [super init]) {

        _errorState = [MTROperationalStateClusterErrorStateStruct new];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTROperationalStateClusterOperationalErrorEvent alloc] init];

    other.errorState = self.errorState;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: errorState:%@; >", NSStringFromClass([self class]), _errorState];
    return descriptionString;
}

@end

@implementation MTROperationalStateClusterOperationCompletionEvent
- (instancetype)init
{
    if (self = [super init]) {

        _completionErrorCode = @(0);

        _totalOperationalTime = nil;

        _pausedTime = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTROperationalStateClusterOperationCompletionEvent alloc] init];

    other.completionErrorCode = self.completionErrorCode;
    other.totalOperationalTime = self.totalOperationalTime;
    other.pausedTime = self.pausedTime;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: completionErrorCode:%@; totalOperationalTime:%@; pausedTime:%@; >", NSStringFromClass([self class]), _completionErrorCode, _totalOperationalTime, _pausedTime];
    return descriptionString;
}

@end

@implementation MTRRVCOperationalStateClusterErrorStateStruct
- (instancetype)init
{
    if (self = [super init]) {

        _errorStateID = @(0);

        _errorStateLabel = nil;

        _errorStateDetails = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRRVCOperationalStateClusterErrorStateStruct alloc] init];

    other.errorStateID = self.errorStateID;
    other.errorStateLabel = self.errorStateLabel;
    other.errorStateDetails = self.errorStateDetails;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: errorStateID:%@; errorStateLabel:%@; errorStateDetails:%@; >", NSStringFromClass([self class]), _errorStateID, _errorStateLabel, _errorStateDetails];
    return descriptionString;
}

@end

@implementation MTRRVCOperationalStateClusterOperationalStateStruct
- (instancetype)init
{
    if (self = [super init]) {

        _operationalStateID = @(0);

        _operationalStateLabel = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRRVCOperationalStateClusterOperationalStateStruct alloc] init];

    other.operationalStateID = self.operationalStateID;
    other.operationalStateLabel = self.operationalStateLabel;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: operationalStateID:%@; operationalStateLabel:%@; >", NSStringFromClass([self class]), _operationalStateID, _operationalStateLabel];
    return descriptionString;
}

@end

@implementation MTRRVCOperationalStateClusterOperationalErrorEvent
- (instancetype)init
{
    if (self = [super init]) {

        _errorState = [MTRRVCOperationalStateClusterErrorStateStruct new];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRRVCOperationalStateClusterOperationalErrorEvent alloc] init];

    other.errorState = self.errorState;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: errorState:%@; >", NSStringFromClass([self class]), _errorState];
    return descriptionString;
}

@end

@implementation MTRRVCOperationalStateClusterOperationCompletionEvent
- (instancetype)init
{
    if (self = [super init]) {

        _completionErrorCode = @(0);

        _totalOperationalTime = nil;

        _pausedTime = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRRVCOperationalStateClusterOperationCompletionEvent alloc] init];

    other.completionErrorCode = self.completionErrorCode;
    other.totalOperationalTime = self.totalOperationalTime;
    other.pausedTime = self.pausedTime;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: completionErrorCode:%@; totalOperationalTime:%@; pausedTime:%@; >", NSStringFromClass([self class]), _completionErrorCode, _totalOperationalTime, _pausedTime];
    return descriptionString;
}

@end

@implementation MTRScenesManagementClusterAttributeValuePairStruct
- (instancetype)init
{
    if (self = [super init]) {

        _attributeID = @(0);

        _valueUnsigned8 = nil;

        _valueSigned8 = nil;

        _valueUnsigned16 = nil;

        _valueSigned16 = nil;

        _valueUnsigned32 = nil;

        _valueSigned32 = nil;

        _valueUnsigned64 = nil;

        _valueSigned64 = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRScenesManagementClusterAttributeValuePairStruct alloc] init];

    other.attributeID = self.attributeID;
    other.valueUnsigned8 = self.valueUnsigned8;
    other.valueSigned8 = self.valueSigned8;
    other.valueUnsigned16 = self.valueUnsigned16;
    other.valueSigned16 = self.valueSigned16;
    other.valueUnsigned32 = self.valueUnsigned32;
    other.valueSigned32 = self.valueSigned32;
    other.valueUnsigned64 = self.valueUnsigned64;
    other.valueSigned64 = self.valueSigned64;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: attributeID:%@; valueUnsigned8:%@; valueSigned8:%@; valueUnsigned16:%@; valueSigned16:%@; valueUnsigned32:%@; valueSigned32:%@; valueUnsigned64:%@; valueSigned64:%@; >", NSStringFromClass([self class]), _attributeID, _valueUnsigned8, _valueSigned8, _valueUnsigned16, _valueSigned16, _valueUnsigned32, _valueSigned32, _valueUnsigned64, _valueSigned64];
    return descriptionString;
}

@end

@implementation MTRScenesManagementClusterExtensionFieldSet
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
    auto other = [[MTRScenesManagementClusterExtensionFieldSet alloc] init];

    other.clusterID = self.clusterID;
    other.attributeValueList = self.attributeValueList;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: clusterID:%@; attributeValueList:%@; >", NSStringFromClass([self class]), _clusterID, _attributeValueList];
    return descriptionString;
}

@end

@implementation MTRScenesManagementClusterSceneInfoStruct
- (instancetype)init
{
    if (self = [super init]) {

        _sceneCount = @(0);

        _currentScene = @(0);

        _currentGroup = @(0);

        _sceneValid = @(0);

        _remainingCapacity = @(0);

        _fabricIndex = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRScenesManagementClusterSceneInfoStruct alloc] init];

    other.sceneCount = self.sceneCount;
    other.currentScene = self.currentScene;
    other.currentGroup = self.currentGroup;
    other.sceneValid = self.sceneValid;
    other.remainingCapacity = self.remainingCapacity;
    other.fabricIndex = self.fabricIndex;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: sceneCount:%@; currentScene:%@; currentGroup:%@; sceneValid:%@; remainingCapacity:%@; fabricIndex:%@; >", NSStringFromClass([self class]), _sceneCount, _currentScene, _currentGroup, _sceneValid, _remainingCapacity, _fabricIndex];
    return descriptionString;
}

@end

@implementation MTRHEPAFilterMonitoringClusterReplacementProductStruct
- (instancetype)init
{
    if (self = [super init]) {

        _productIdentifierType = @(0);

        _productIdentifierValue = @"";
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRHEPAFilterMonitoringClusterReplacementProductStruct alloc] init];

    other.productIdentifierType = self.productIdentifierType;
    other.productIdentifierValue = self.productIdentifierValue;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: productIdentifierType:%@; productIdentifierValue:%@; >", NSStringFromClass([self class]), _productIdentifierType, _productIdentifierValue];
    return descriptionString;
}

@end

@implementation MTRActivatedCarbonFilterMonitoringClusterReplacementProductStruct
- (instancetype)init
{
    if (self = [super init]) {

        _productIdentifierType = @(0);

        _productIdentifierValue = @"";
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRActivatedCarbonFilterMonitoringClusterReplacementProductStruct alloc] init];

    other.productIdentifierType = self.productIdentifierType;
    other.productIdentifierValue = self.productIdentifierValue;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: productIdentifierType:%@; productIdentifierValue:%@; >", NSStringFromClass([self class]), _productIdentifierType, _productIdentifierValue];
    return descriptionString;
}

@end

@implementation MTRBooleanStateConfigurationClusterAlarmsStateChangedEvent
- (instancetype)init
{
    if (self = [super init]) {

        _alarmsActive = @(0);

        _alarmsSuppressed = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRBooleanStateConfigurationClusterAlarmsStateChangedEvent alloc] init];

    other.alarmsActive = self.alarmsActive;
    other.alarmsSuppressed = self.alarmsSuppressed;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: alarmsActive:%@; alarmsSuppressed:%@; >", NSStringFromClass([self class]), _alarmsActive, _alarmsSuppressed];
    return descriptionString;
}

@end

@implementation MTRBooleanStateConfigurationClusterSensorFaultEvent
- (instancetype)init
{
    if (self = [super init]) {

        _sensorFault = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRBooleanStateConfigurationClusterSensorFaultEvent alloc] init];

    other.sensorFault = self.sensorFault;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: sensorFault:%@; >", NSStringFromClass([self class]), _sensorFault];
    return descriptionString;
}

@end

@implementation MTRValveConfigurationAndControlClusterValveStateChangedEvent
- (instancetype)init
{
    if (self = [super init]) {

        _valveState = @(0);

        _valveLevel = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRValveConfigurationAndControlClusterValveStateChangedEvent alloc] init];

    other.valveState = self.valveState;
    other.valveLevel = self.valveLevel;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: valveState:%@; valveLevel:%@; >", NSStringFromClass([self class]), _valveState, _valveLevel];
    return descriptionString;
}

@end

@implementation MTRValveConfigurationAndControlClusterValveFaultEvent
- (instancetype)init
{
    if (self = [super init]) {

        _valveFault = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRValveConfigurationAndControlClusterValveFaultEvent alloc] init];

    other.valveFault = self.valveFault;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: valveFault:%@; >", NSStringFromClass([self class]), _valveFault];
    return descriptionString;
}

@end

@implementation MTRElectricalPowerMeasurementClusterMeasurementAccuracyRangeStruct
- (instancetype)init
{
    if (self = [super init]) {

        _rangeMin = @(0);

        _rangeMax = @(0);

        _percentMax = nil;

        _percentMin = nil;

        _percentTypical = nil;

        _fixedMax = nil;

        _fixedMin = nil;

        _fixedTypical = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRElectricalPowerMeasurementClusterMeasurementAccuracyRangeStruct alloc] init];

    other.rangeMin = self.rangeMin;
    other.rangeMax = self.rangeMax;
    other.percentMax = self.percentMax;
    other.percentMin = self.percentMin;
    other.percentTypical = self.percentTypical;
    other.fixedMax = self.fixedMax;
    other.fixedMin = self.fixedMin;
    other.fixedTypical = self.fixedTypical;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: rangeMin:%@; rangeMax:%@; percentMax:%@; percentMin:%@; percentTypical:%@; fixedMax:%@; fixedMin:%@; fixedTypical:%@; >", NSStringFromClass([self class]), _rangeMin, _rangeMax, _percentMax, _percentMin, _percentTypical, _fixedMax, _fixedMin, _fixedTypical];
    return descriptionString;
}

@end

@implementation MTRElectricalPowerMeasurementClusterMeasurementAccuracyStruct
- (instancetype)init
{
    if (self = [super init]) {

        _measurementType = @(0);

        _measured = @(0);

        _minMeasuredValue = @(0);

        _maxMeasuredValue = @(0);

        _accuracyRanges = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRElectricalPowerMeasurementClusterMeasurementAccuracyStruct alloc] init];

    other.measurementType = self.measurementType;
    other.measured = self.measured;
    other.minMeasuredValue = self.minMeasuredValue;
    other.maxMeasuredValue = self.maxMeasuredValue;
    other.accuracyRanges = self.accuracyRanges;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: measurementType:%@; measured:%@; minMeasuredValue:%@; maxMeasuredValue:%@; accuracyRanges:%@; >", NSStringFromClass([self class]), _measurementType, _measured, _minMeasuredValue, _maxMeasuredValue, _accuracyRanges];
    return descriptionString;
}

@end

@implementation MTRElectricalPowerMeasurementClusterHarmonicMeasurementStruct
- (instancetype)init
{
    if (self = [super init]) {

        _order = @(0);

        _measurement = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRElectricalPowerMeasurementClusterHarmonicMeasurementStruct alloc] init];

    other.order = self.order;
    other.measurement = self.measurement;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: order:%@; measurement:%@; >", NSStringFromClass([self class]), _order, _measurement];
    return descriptionString;
}

@end

@implementation MTRElectricalPowerMeasurementClusterMeasurementRangeStruct
- (instancetype)init
{
    if (self = [super init]) {

        _measurementType = @(0);

        _min = @(0);

        _max = @(0);

        _startTimestamp = nil;

        _endTimestamp = nil;

        _minTimestamp = nil;

        _maxTimestamp = nil;

        _startSystime = nil;

        _endSystime = nil;

        _minSystime = nil;

        _maxSystime = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRElectricalPowerMeasurementClusterMeasurementRangeStruct alloc] init];

    other.measurementType = self.measurementType;
    other.min = self.min;
    other.max = self.max;
    other.startTimestamp = self.startTimestamp;
    other.endTimestamp = self.endTimestamp;
    other.minTimestamp = self.minTimestamp;
    other.maxTimestamp = self.maxTimestamp;
    other.startSystime = self.startSystime;
    other.endSystime = self.endSystime;
    other.minSystime = self.minSystime;
    other.maxSystime = self.maxSystime;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: measurementType:%@; min:%@; max:%@; startTimestamp:%@; endTimestamp:%@; minTimestamp:%@; maxTimestamp:%@; startSystime:%@; endSystime:%@; minSystime:%@; maxSystime:%@; >", NSStringFromClass([self class]), _measurementType, _min, _max, _startTimestamp, _endTimestamp, _minTimestamp, _maxTimestamp, _startSystime, _endSystime, _minSystime, _maxSystime];
    return descriptionString;
}

@end

@implementation MTRElectricalPowerMeasurementClusterMeasurementPeriodRangesEvent
- (instancetype)init
{
    if (self = [super init]) {

        _ranges = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRElectricalPowerMeasurementClusterMeasurementPeriodRangesEvent alloc] init];

    other.ranges = self.ranges;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: ranges:%@; >", NSStringFromClass([self class]), _ranges];
    return descriptionString;
}

@end

@implementation MTRElectricalEnergyMeasurementClusterMeasurementAccuracyRangeStruct
- (instancetype)init
{
    if (self = [super init]) {

        _rangeMin = @(0);

        _rangeMax = @(0);

        _percentMax = nil;

        _percentMin = nil;

        _percentTypical = nil;

        _fixedMax = nil;

        _fixedMin = nil;

        _fixedTypical = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRElectricalEnergyMeasurementClusterMeasurementAccuracyRangeStruct alloc] init];

    other.rangeMin = self.rangeMin;
    other.rangeMax = self.rangeMax;
    other.percentMax = self.percentMax;
    other.percentMin = self.percentMin;
    other.percentTypical = self.percentTypical;
    other.fixedMax = self.fixedMax;
    other.fixedMin = self.fixedMin;
    other.fixedTypical = self.fixedTypical;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: rangeMin:%@; rangeMax:%@; percentMax:%@; percentMin:%@; percentTypical:%@; fixedMax:%@; fixedMin:%@; fixedTypical:%@; >", NSStringFromClass([self class]), _rangeMin, _rangeMax, _percentMax, _percentMin, _percentTypical, _fixedMax, _fixedMin, _fixedTypical];
    return descriptionString;
}

@end

@implementation MTRElectricalEnergyMeasurementClusterMeasurementAccuracyStruct
- (instancetype)init
{
    if (self = [super init]) {

        _measurementType = @(0);

        _measured = @(0);

        _minMeasuredValue = @(0);

        _maxMeasuredValue = @(0);

        _accuracyRanges = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRElectricalEnergyMeasurementClusterMeasurementAccuracyStruct alloc] init];

    other.measurementType = self.measurementType;
    other.measured = self.measured;
    other.minMeasuredValue = self.minMeasuredValue;
    other.maxMeasuredValue = self.maxMeasuredValue;
    other.accuracyRanges = self.accuracyRanges;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: measurementType:%@; measured:%@; minMeasuredValue:%@; maxMeasuredValue:%@; accuracyRanges:%@; >", NSStringFromClass([self class]), _measurementType, _measured, _minMeasuredValue, _maxMeasuredValue, _accuracyRanges];
    return descriptionString;
}

@end

@implementation MTRElectricalEnergyMeasurementClusterCumulativeEnergyResetStruct
- (instancetype)init
{
    if (self = [super init]) {

        _importedResetTimestamp = nil;

        _exportedResetTimestamp = nil;

        _importedResetSystime = nil;

        _exportedResetSystime = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRElectricalEnergyMeasurementClusterCumulativeEnergyResetStruct alloc] init];

    other.importedResetTimestamp = self.importedResetTimestamp;
    other.exportedResetTimestamp = self.exportedResetTimestamp;
    other.importedResetSystime = self.importedResetSystime;
    other.exportedResetSystime = self.exportedResetSystime;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: importedResetTimestamp:%@; exportedResetTimestamp:%@; importedResetSystime:%@; exportedResetSystime:%@; >", NSStringFromClass([self class]), _importedResetTimestamp, _exportedResetTimestamp, _importedResetSystime, _exportedResetSystime];
    return descriptionString;
}

@end

@implementation MTRElectricalEnergyMeasurementClusterEnergyMeasurementStruct
- (instancetype)init
{
    if (self = [super init]) {

        _energy = @(0);

        _startTimestamp = nil;

        _endTimestamp = nil;

        _startSystime = nil;

        _endSystime = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRElectricalEnergyMeasurementClusterEnergyMeasurementStruct alloc] init];

    other.energy = self.energy;
    other.startTimestamp = self.startTimestamp;
    other.endTimestamp = self.endTimestamp;
    other.startSystime = self.startSystime;
    other.endSystime = self.endSystime;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: energy:%@; startTimestamp:%@; endTimestamp:%@; startSystime:%@; endSystime:%@; >", NSStringFromClass([self class]), _energy, _startTimestamp, _endTimestamp, _startSystime, _endSystime];
    return descriptionString;
}

@end

@implementation MTRElectricalEnergyMeasurementClusterCumulativeEnergyMeasuredEvent
- (instancetype)init
{
    if (self = [super init]) {

        _energyImported = nil;

        _energyExported = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRElectricalEnergyMeasurementClusterCumulativeEnergyMeasuredEvent alloc] init];

    other.energyImported = self.energyImported;
    other.energyExported = self.energyExported;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: energyImported:%@; energyExported:%@; >", NSStringFromClass([self class]), _energyImported, _energyExported];
    return descriptionString;
}

@end

@implementation MTRElectricalEnergyMeasurementClusterPeriodicEnergyMeasuredEvent
- (instancetype)init
{
    if (self = [super init]) {

        _energyImported = nil;

        _energyExported = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRElectricalEnergyMeasurementClusterPeriodicEnergyMeasuredEvent alloc] init];

    other.energyImported = self.energyImported;
    other.energyExported = self.energyExported;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: energyImported:%@; energyExported:%@; >", NSStringFromClass([self class]), _energyImported, _energyExported];
    return descriptionString;
}

@end

@implementation MTRWaterHeaterManagementClusterWaterHeaterBoostInfoStruct
- (instancetype)init
{
    if (self = [super init]) {

        _duration = @(0);

        _oneShot = nil;

        _emergencyBoost = nil;

        _temporarySetpoint = nil;

        _targetPercentage = nil;

        _targetReheat = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRWaterHeaterManagementClusterWaterHeaterBoostInfoStruct alloc] init];

    other.duration = self.duration;
    other.oneShot = self.oneShot;
    other.emergencyBoost = self.emergencyBoost;
    other.temporarySetpoint = self.temporarySetpoint;
    other.targetPercentage = self.targetPercentage;
    other.targetReheat = self.targetReheat;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: duration:%@; oneShot:%@; emergencyBoost:%@; temporarySetpoint:%@; targetPercentage:%@; targetReheat:%@; >", NSStringFromClass([self class]), _duration, _oneShot, _emergencyBoost, _temporarySetpoint, _targetPercentage, _targetReheat];
    return descriptionString;
}

@end

@implementation MTRWaterHeaterManagementClusterBoostStartedEvent
- (instancetype)init
{
    if (self = [super init]) {

        _boostInfo = [MTRWaterHeaterManagementClusterWaterHeaterBoostInfoStruct new];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRWaterHeaterManagementClusterBoostStartedEvent alloc] init];

    other.boostInfo = self.boostInfo;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: boostInfo:%@; >", NSStringFromClass([self class]), _boostInfo];
    return descriptionString;
}

@end

@implementation MTRWaterHeaterManagementClusterBoostEndedEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRWaterHeaterManagementClusterBoostEndedEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRDemandResponseLoadControlClusterHeatingSourceControlStruct
- (instancetype)init
{
    if (self = [super init]) {

        _heatingSource = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDemandResponseLoadControlClusterHeatingSourceControlStruct alloc] init];

    other.heatingSource = self.heatingSource;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: heatingSource:%@; >", NSStringFromClass([self class]), _heatingSource];
    return descriptionString;
}

@end

@implementation MTRDemandResponseLoadControlClusterPowerSavingsControlStruct
- (instancetype)init
{
    if (self = [super init]) {

        _powerSavings = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDemandResponseLoadControlClusterPowerSavingsControlStruct alloc] init];

    other.powerSavings = self.powerSavings;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: powerSavings:%@; >", NSStringFromClass([self class]), _powerSavings];
    return descriptionString;
}

@end

@implementation MTRDemandResponseLoadControlClusterDutyCycleControlStruct
- (instancetype)init
{
    if (self = [super init]) {

        _dutyCycle = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDemandResponseLoadControlClusterDutyCycleControlStruct alloc] init];

    other.dutyCycle = self.dutyCycle;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: dutyCycle:%@; >", NSStringFromClass([self class]), _dutyCycle];
    return descriptionString;
}

@end

@implementation MTRDemandResponseLoadControlClusterAverageLoadControlStruct
- (instancetype)init
{
    if (self = [super init]) {

        _loadAdjustment = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDemandResponseLoadControlClusterAverageLoadControlStruct alloc] init];

    other.loadAdjustment = self.loadAdjustment;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: loadAdjustment:%@; >", NSStringFromClass([self class]), _loadAdjustment];
    return descriptionString;
}

@end

@implementation MTRDemandResponseLoadControlClusterTemperatureControlStruct
- (instancetype)init
{
    if (self = [super init]) {

        _coolingTempOffset = nil;

        _heatingtTempOffset = nil;

        _coolingTempSetpoint = nil;

        _heatingTempSetpoint = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDemandResponseLoadControlClusterTemperatureControlStruct alloc] init];

    other.coolingTempOffset = self.coolingTempOffset;
    other.heatingtTempOffset = self.heatingtTempOffset;
    other.coolingTempSetpoint = self.coolingTempSetpoint;
    other.heatingTempSetpoint = self.heatingTempSetpoint;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: coolingTempOffset:%@; heatingtTempOffset:%@; coolingTempSetpoint:%@; heatingTempSetpoint:%@; >", NSStringFromClass([self class]), _coolingTempOffset, _heatingtTempOffset, _coolingTempSetpoint, _heatingTempSetpoint];
    return descriptionString;
}

@end

@implementation MTRDemandResponseLoadControlClusterLoadControlEventTransitionStruct
- (instancetype)init
{
    if (self = [super init]) {

        _duration = @(0);

        _control = @(0);

        _temperatureControl = nil;

        _averageLoadControl = nil;

        _dutyCycleControl = nil;

        _powerSavingsControl = nil;

        _heatingSourceControl = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDemandResponseLoadControlClusterLoadControlEventTransitionStruct alloc] init];

    other.duration = self.duration;
    other.control = self.control;
    other.temperatureControl = self.temperatureControl;
    other.averageLoadControl = self.averageLoadControl;
    other.dutyCycleControl = self.dutyCycleControl;
    other.powerSavingsControl = self.powerSavingsControl;
    other.heatingSourceControl = self.heatingSourceControl;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: duration:%@; control:%@; temperatureControl:%@; averageLoadControl:%@; dutyCycleControl:%@; powerSavingsControl:%@; heatingSourceControl:%@; >", NSStringFromClass([self class]), _duration, _control, _temperatureControl, _averageLoadControl, _dutyCycleControl, _powerSavingsControl, _heatingSourceControl];
    return descriptionString;
}

@end

@implementation MTRDemandResponseLoadControlClusterLoadControlEventStruct
- (instancetype)init
{
    if (self = [super init]) {

        _eventID = [NSData data];

        _programID = nil;

        _control = @(0);

        _deviceClass = @(0);

        _enrollmentGroup = nil;

        _criticality = @(0);

        _startTime = nil;

        _transitions = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDemandResponseLoadControlClusterLoadControlEventStruct alloc] init];

    other.eventID = self.eventID;
    other.programID = self.programID;
    other.control = self.control;
    other.deviceClass = self.deviceClass;
    other.enrollmentGroup = self.enrollmentGroup;
    other.criticality = self.criticality;
    other.startTime = self.startTime;
    other.transitions = self.transitions;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: eventID:%@; programID:%@; control:%@; deviceClass:%@; enrollmentGroup:%@; criticality:%@; startTime:%@; transitions:%@; >", NSStringFromClass([self class]), [_eventID base64EncodedStringWithOptions:0], [_programID base64EncodedStringWithOptions:0], _control, _deviceClass, _enrollmentGroup, _criticality, _startTime, _transitions];
    return descriptionString;
}

@end

@implementation MTRDemandResponseLoadControlClusterLoadControlProgramStruct
- (instancetype)init
{
    if (self = [super init]) {

        _programID = [NSData data];

        _name = @"";

        _enrollmentGroup = nil;

        _randomStartMinutes = nil;

        _randomDurationMinutes = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDemandResponseLoadControlClusterLoadControlProgramStruct alloc] init];

    other.programID = self.programID;
    other.name = self.name;
    other.enrollmentGroup = self.enrollmentGroup;
    other.randomStartMinutes = self.randomStartMinutes;
    other.randomDurationMinutes = self.randomDurationMinutes;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: programID:%@; name:%@; enrollmentGroup:%@; randomStartMinutes:%@; randomDurationMinutes:%@; >", NSStringFromClass([self class]), [_programID base64EncodedStringWithOptions:0], _name, _enrollmentGroup, _randomStartMinutes, _randomDurationMinutes];
    return descriptionString;
}

@end

@implementation MTRDemandResponseLoadControlClusterLoadControlEventStatusChangeEvent
- (instancetype)init
{
    if (self = [super init]) {

        _eventID = [NSData data];

        _transitionIndex = nil;

        _status = @(0);

        _criticality = @(0);

        _control = @(0);

        _temperatureControl = nil;

        _averageLoadControl = nil;

        _dutyCycleControl = nil;

        _powerSavingsControl = nil;

        _heatingSourceControl = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDemandResponseLoadControlClusterLoadControlEventStatusChangeEvent alloc] init];

    other.eventID = self.eventID;
    other.transitionIndex = self.transitionIndex;
    other.status = self.status;
    other.criticality = self.criticality;
    other.control = self.control;
    other.temperatureControl = self.temperatureControl;
    other.averageLoadControl = self.averageLoadControl;
    other.dutyCycleControl = self.dutyCycleControl;
    other.powerSavingsControl = self.powerSavingsControl;
    other.heatingSourceControl = self.heatingSourceControl;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: eventID:%@; transitionIndex:%@; status:%@; criticality:%@; control:%@; temperatureControl:%@; averageLoadControl:%@; dutyCycleControl:%@; powerSavingsControl:%@; heatingSourceControl:%@; >", NSStringFromClass([self class]), [_eventID base64EncodedStringWithOptions:0], _transitionIndex, _status, _criticality, _control, _temperatureControl, _averageLoadControl, _dutyCycleControl, _powerSavingsControl, _heatingSourceControl];
    return descriptionString;
}

@end

@implementation MTRMessagesClusterMessageResponseOptionStruct
- (instancetype)init
{
    if (self = [super init]) {

        _messageResponseID = nil;

        _label = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRMessagesClusterMessageResponseOptionStruct alloc] init];

    other.messageResponseID = self.messageResponseID;
    other.label = self.label;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: messageResponseID:%@; label:%@; >", NSStringFromClass([self class]), _messageResponseID, _label];
    return descriptionString;
}

@end

@implementation MTRMessagesClusterMessageStruct
- (instancetype)init
{
    if (self = [super init]) {

        _messageID = [NSData data];

        _priority = @(0);

        _messageControl = @(0);

        _startTime = nil;

        _duration = nil;

        _messageText = @"";

        _responses = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRMessagesClusterMessageStruct alloc] init];

    other.messageID = self.messageID;
    other.priority = self.priority;
    other.messageControl = self.messageControl;
    other.startTime = self.startTime;
    other.duration = self.duration;
    other.messageText = self.messageText;
    other.responses = self.responses;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: messageID:%@; priority:%@; messageControl:%@; startTime:%@; duration:%@; messageText:%@; responses:%@; >", NSStringFromClass([self class]), [_messageID base64EncodedStringWithOptions:0], _priority, _messageControl, _startTime, _duration, _messageText, _responses];
    return descriptionString;
}

@end

@implementation MTRMessagesClusterMessageQueuedEvent
- (instancetype)init
{
    if (self = [super init]) {

        _messageID = [NSData data];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRMessagesClusterMessageQueuedEvent alloc] init];

    other.messageID = self.messageID;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: messageID:%@; >", NSStringFromClass([self class]), [_messageID base64EncodedStringWithOptions:0]];
    return descriptionString;
}

@end

@implementation MTRMessagesClusterMessagePresentedEvent
- (instancetype)init
{
    if (self = [super init]) {

        _messageID = [NSData data];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRMessagesClusterMessagePresentedEvent alloc] init];

    other.messageID = self.messageID;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: messageID:%@; >", NSStringFromClass([self class]), [_messageID base64EncodedStringWithOptions:0]];
    return descriptionString;
}

@end

@implementation MTRMessagesClusterMessageCompleteEvent
- (instancetype)init
{
    if (self = [super init]) {

        _messageID = [NSData data];

        _responseID = nil;

        _reply = nil;

        _futureMessagesPreference = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRMessagesClusterMessageCompleteEvent alloc] init];

    other.messageID = self.messageID;
    other.responseID = self.responseID;
    other.reply = self.reply;
    other.futureMessagesPreference = self.futureMessagesPreference;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: messageID:%@; responseID:%@; reply:%@; futureMessagesPreference:%@; >", NSStringFromClass([self class]), [_messageID base64EncodedStringWithOptions:0], _responseID, _reply, _futureMessagesPreference];
    return descriptionString;
}

@end

@implementation MTRDeviceEnergyManagementClusterCostStruct
- (instancetype)init
{
    if (self = [super init]) {

        _costType = @(0);

        _value = @(0);

        _decimalPoints = @(0);

        _currency = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDeviceEnergyManagementClusterCostStruct alloc] init];

    other.costType = self.costType;
    other.value = self.value;
    other.decimalPoints = self.decimalPoints;
    other.currency = self.currency;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: costType:%@; value:%@; decimalPoints:%@; currency:%@; >", NSStringFromClass([self class]), _costType, _value, _decimalPoints, _currency];
    return descriptionString;
}

@end

@implementation MTRDeviceEnergyManagementClusterPowerAdjustStruct
- (instancetype)init
{
    if (self = [super init]) {

        _minPower = @(0);

        _maxPower = @(0);

        _minDuration = @(0);

        _maxDuration = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDeviceEnergyManagementClusterPowerAdjustStruct alloc] init];

    other.minPower = self.minPower;
    other.maxPower = self.maxPower;
    other.minDuration = self.minDuration;
    other.maxDuration = self.maxDuration;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: minPower:%@; maxPower:%@; minDuration:%@; maxDuration:%@; >", NSStringFromClass([self class]), _minPower, _maxPower, _minDuration, _maxDuration];
    return descriptionString;
}

@end

@implementation MTRDeviceEnergyManagementClusterPowerAdjustCapabilityStruct
- (instancetype)init
{
    if (self = [super init]) {

        _powerAdjustCapability = nil;

        _cause = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDeviceEnergyManagementClusterPowerAdjustCapabilityStruct alloc] init];

    other.powerAdjustCapability = self.powerAdjustCapability;
    other.cause = self.cause;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: powerAdjustCapability:%@; cause:%@; >", NSStringFromClass([self class]), _powerAdjustCapability, _cause];
    return descriptionString;
}

@end

@implementation MTRDeviceEnergyManagementClusterSlotStruct
- (instancetype)init
{
    if (self = [super init]) {

        _minDuration = @(0);

        _maxDuration = @(0);

        _defaultDuration = @(0);

        _elapsedSlotTime = @(0);

        _remainingSlotTime = @(0);

        _slotIsPausable = nil;

        _minPauseDuration = nil;

        _maxPauseDuration = nil;

        _manufacturerESAState = nil;

        _nominalPower = nil;

        _minPower = nil;

        _maxPower = nil;

        _nominalEnergy = nil;

        _costs = nil;

        _minPowerAdjustment = nil;

        _maxPowerAdjustment = nil;

        _minDurationAdjustment = nil;

        _maxDurationAdjustment = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDeviceEnergyManagementClusterSlotStruct alloc] init];

    other.minDuration = self.minDuration;
    other.maxDuration = self.maxDuration;
    other.defaultDuration = self.defaultDuration;
    other.elapsedSlotTime = self.elapsedSlotTime;
    other.remainingSlotTime = self.remainingSlotTime;
    other.slotIsPausable = self.slotIsPausable;
    other.minPauseDuration = self.minPauseDuration;
    other.maxPauseDuration = self.maxPauseDuration;
    other.manufacturerESAState = self.manufacturerESAState;
    other.nominalPower = self.nominalPower;
    other.minPower = self.minPower;
    other.maxPower = self.maxPower;
    other.nominalEnergy = self.nominalEnergy;
    other.costs = self.costs;
    other.minPowerAdjustment = self.minPowerAdjustment;
    other.maxPowerAdjustment = self.maxPowerAdjustment;
    other.minDurationAdjustment = self.minDurationAdjustment;
    other.maxDurationAdjustment = self.maxDurationAdjustment;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: minDuration:%@; maxDuration:%@; defaultDuration:%@; elapsedSlotTime:%@; remainingSlotTime:%@; slotIsPausable:%@; minPauseDuration:%@; maxPauseDuration:%@; manufacturerESAState:%@; nominalPower:%@; minPower:%@; maxPower:%@; nominalEnergy:%@; costs:%@; minPowerAdjustment:%@; maxPowerAdjustment:%@; minDurationAdjustment:%@; maxDurationAdjustment:%@; >", NSStringFromClass([self class]), _minDuration, _maxDuration, _defaultDuration, _elapsedSlotTime, _remainingSlotTime, _slotIsPausable, _minPauseDuration, _maxPauseDuration, _manufacturerESAState, _nominalPower, _minPower, _maxPower, _nominalEnergy, _costs, _minPowerAdjustment, _maxPowerAdjustment, _minDurationAdjustment, _maxDurationAdjustment];
    return descriptionString;
}

@end

@implementation MTRDeviceEnergyManagementClusterForecastStruct
- (instancetype)init
{
    if (self = [super init]) {

        _forecastID = @(0);

        _activeSlotNumber = nil;

        _startTime = @(0);

        _endTime = @(0);

        _earliestStartTime = nil;

        _latestEndTime = nil;

        _isPausable = @(0);

        _slots = [NSArray array];

        _forecastUpdateReason = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDeviceEnergyManagementClusterForecastStruct alloc] init];

    other.forecastID = self.forecastID;
    other.activeSlotNumber = self.activeSlotNumber;
    other.startTime = self.startTime;
    other.endTime = self.endTime;
    other.earliestStartTime = self.earliestStartTime;
    other.latestEndTime = self.latestEndTime;
    other.isPausable = self.isPausable;
    other.slots = self.slots;
    other.forecastUpdateReason = self.forecastUpdateReason;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: forecastID:%@; activeSlotNumber:%@; startTime:%@; endTime:%@; earliestStartTime:%@; latestEndTime:%@; isPausable:%@; slots:%@; forecastUpdateReason:%@; >", NSStringFromClass([self class]), _forecastID, _activeSlotNumber, _startTime, _endTime, _earliestStartTime, _latestEndTime, _isPausable, _slots, _forecastUpdateReason];
    return descriptionString;
}

@end

@implementation MTRDeviceEnergyManagementClusterConstraintsStruct
- (instancetype)init
{
    if (self = [super init]) {

        _startTime = @(0);

        _duration = @(0);

        _nominalPower = nil;

        _maximumEnergy = nil;

        _loadControl = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDeviceEnergyManagementClusterConstraintsStruct alloc] init];

    other.startTime = self.startTime;
    other.duration = self.duration;
    other.nominalPower = self.nominalPower;
    other.maximumEnergy = self.maximumEnergy;
    other.loadControl = self.loadControl;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: startTime:%@; duration:%@; nominalPower:%@; maximumEnergy:%@; loadControl:%@; >", NSStringFromClass([self class]), _startTime, _duration, _nominalPower, _maximumEnergy, _loadControl];
    return descriptionString;
}

@end

@implementation MTRDeviceEnergyManagementClusterSlotAdjustmentStruct
- (instancetype)init
{
    if (self = [super init]) {

        _slotIndex = @(0);

        _nominalPower = nil;

        _duration = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDeviceEnergyManagementClusterSlotAdjustmentStruct alloc] init];

    other.slotIndex = self.slotIndex;
    other.nominalPower = self.nominalPower;
    other.duration = self.duration;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: slotIndex:%@; nominalPower:%@; duration:%@; >", NSStringFromClass([self class]), _slotIndex, _nominalPower, _duration];
    return descriptionString;
}

@end

@implementation MTRDeviceEnergyManagementClusterPowerAdjustStartEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDeviceEnergyManagementClusterPowerAdjustStartEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRDeviceEnergyManagementClusterPowerAdjustEndEvent
- (instancetype)init
{
    if (self = [super init]) {

        _cause = @(0);

        _duration = @(0);

        _energyUse = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDeviceEnergyManagementClusterPowerAdjustEndEvent alloc] init];

    other.cause = self.cause;
    other.duration = self.duration;
    other.energyUse = self.energyUse;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: cause:%@; duration:%@; energyUse:%@; >", NSStringFromClass([self class]), _cause, _duration, _energyUse];
    return descriptionString;
}

@end

@implementation MTRDeviceEnergyManagementClusterPausedEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDeviceEnergyManagementClusterPausedEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRDeviceEnergyManagementClusterResumedEvent
- (instancetype)init
{
    if (self = [super init]) {

        _cause = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDeviceEnergyManagementClusterResumedEvent alloc] init];

    other.cause = self.cause;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: cause:%@; >", NSStringFromClass([self class]), _cause];
    return descriptionString;
}

@end

@implementation MTREnergyEVSEClusterChargingTargetStruct
- (instancetype)init
{
    if (self = [super init]) {

        _targetTimeMinutesPastMidnight = @(0);

        _targetSoC = nil;

        _addedEnergy = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTREnergyEVSEClusterChargingTargetStruct alloc] init];

    other.targetTimeMinutesPastMidnight = self.targetTimeMinutesPastMidnight;
    other.targetSoC = self.targetSoC;
    other.addedEnergy = self.addedEnergy;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: targetTimeMinutesPastMidnight:%@; targetSoC:%@; addedEnergy:%@; >", NSStringFromClass([self class]), _targetTimeMinutesPastMidnight, _targetSoC, _addedEnergy];
    return descriptionString;
}

@end

@implementation MTREnergyEVSEClusterChargingTargetScheduleStruct
- (instancetype)init
{
    if (self = [super init]) {

        _dayOfWeekForSequence = @(0);

        _chargingTargets = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTREnergyEVSEClusterChargingTargetScheduleStruct alloc] init];

    other.dayOfWeekForSequence = self.dayOfWeekForSequence;
    other.chargingTargets = self.chargingTargets;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: dayOfWeekForSequence:%@; chargingTargets:%@; >", NSStringFromClass([self class]), _dayOfWeekForSequence, _chargingTargets];
    return descriptionString;
}

@end

@implementation MTREnergyEVSEClusterEVConnectedEvent
- (instancetype)init
{
    if (self = [super init]) {

        _sessionID = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTREnergyEVSEClusterEVConnectedEvent alloc] init];

    other.sessionID = self.sessionID;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: sessionID:%@; >", NSStringFromClass([self class]), _sessionID];
    return descriptionString;
}

@end

@implementation MTREnergyEVSEClusterEVNotDetectedEvent
- (instancetype)init
{
    if (self = [super init]) {

        _sessionID = @(0);

        _state = @(0);

        _sessionDuration = @(0);

        _sessionEnergyCharged = @(0);

        _sessionEnergyDischarged = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTREnergyEVSEClusterEVNotDetectedEvent alloc] init];

    other.sessionID = self.sessionID;
    other.state = self.state;
    other.sessionDuration = self.sessionDuration;
    other.sessionEnergyCharged = self.sessionEnergyCharged;
    other.sessionEnergyDischarged = self.sessionEnergyDischarged;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: sessionID:%@; state:%@; sessionDuration:%@; sessionEnergyCharged:%@; sessionEnergyDischarged:%@; >", NSStringFromClass([self class]), _sessionID, _state, _sessionDuration, _sessionEnergyCharged, _sessionEnergyDischarged];
    return descriptionString;
}

@end

@implementation MTREnergyEVSEClusterEnergyTransferStartedEvent
- (instancetype)init
{
    if (self = [super init]) {

        _sessionID = @(0);

        _state = @(0);

        _maximumCurrent = @(0);

        _maximumDischargeCurrent = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTREnergyEVSEClusterEnergyTransferStartedEvent alloc] init];

    other.sessionID = self.sessionID;
    other.state = self.state;
    other.maximumCurrent = self.maximumCurrent;
    other.maximumDischargeCurrent = self.maximumDischargeCurrent;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: sessionID:%@; state:%@; maximumCurrent:%@; maximumDischargeCurrent:%@; >", NSStringFromClass([self class]), _sessionID, _state, _maximumCurrent, _maximumDischargeCurrent];
    return descriptionString;
}

@end

@implementation MTREnergyEVSEClusterEnergyTransferStoppedEvent
- (instancetype)init
{
    if (self = [super init]) {

        _sessionID = @(0);

        _state = @(0);

        _reason = @(0);

        _energyTransferred = @(0);

        _energyDischarged = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTREnergyEVSEClusterEnergyTransferStoppedEvent alloc] init];

    other.sessionID = self.sessionID;
    other.state = self.state;
    other.reason = self.reason;
    other.energyTransferred = self.energyTransferred;
    other.energyDischarged = self.energyDischarged;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: sessionID:%@; state:%@; reason:%@; energyTransferred:%@; energyDischarged:%@; >", NSStringFromClass([self class]), _sessionID, _state, _reason, _energyTransferred, _energyDischarged];
    return descriptionString;
}

@end

@implementation MTREnergyEVSEClusterFaultEvent
- (instancetype)init
{
    if (self = [super init]) {

        _sessionID = nil;

        _state = @(0);

        _faultStatePreviousState = @(0);

        _faultStateCurrentState = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTREnergyEVSEClusterFaultEvent alloc] init];

    other.sessionID = self.sessionID;
    other.state = self.state;
    other.faultStatePreviousState = self.faultStatePreviousState;
    other.faultStateCurrentState = self.faultStateCurrentState;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: sessionID:%@; state:%@; faultStatePreviousState:%@; faultStateCurrentState:%@; >", NSStringFromClass([self class]), _sessionID, _state, _faultStatePreviousState, _faultStateCurrentState];
    return descriptionString;
}

@end

@implementation MTREnergyEVSEClusterRFIDEvent
- (instancetype)init
{
    if (self = [super init]) {

        _uid = [NSData data];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTREnergyEVSEClusterRFIDEvent alloc] init];

    other.uid = self.uid;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: uid:%@; >", NSStringFromClass([self class]), [_uid base64EncodedStringWithOptions:0]];
    return descriptionString;
}

@end

@implementation MTREnergyPreferenceClusterBalanceStruct
- (instancetype)init
{
    if (self = [super init]) {

        _step = @(0);

        _label = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTREnergyPreferenceClusterBalanceStruct alloc] init];

    other.step = self.step;
    other.label = self.label;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: step:%@; label:%@; >", NSStringFromClass([self class]), _step, _label];
    return descriptionString;
}

@end

@implementation MTREnergyEVSEModeClusterModeTagStruct
- (instancetype)init
{
    if (self = [super init]) {

        _mfgCode = nil;

        _value = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTREnergyEVSEModeClusterModeTagStruct alloc] init];

    other.mfgCode = self.mfgCode;
    other.value = self.value;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: mfgCode:%@; value:%@; >", NSStringFromClass([self class]), _mfgCode, _value];
    return descriptionString;
}

@end

@implementation MTREnergyEVSEModeClusterModeOptionStruct
- (instancetype)init
{
    if (self = [super init]) {

        _label = @"";

        _mode = @(0);

        _modeTags = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTREnergyEVSEModeClusterModeOptionStruct alloc] init];

    other.label = self.label;
    other.mode = self.mode;
    other.modeTags = self.modeTags;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: label:%@; mode:%@; modeTags:%@; >", NSStringFromClass([self class]), _label, _mode, _modeTags];
    return descriptionString;
}

@end

@implementation MTRWaterHeaterModeClusterModeTagStruct
- (instancetype)init
{
    if (self = [super init]) {

        _mfgCode = nil;

        _value = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRWaterHeaterModeClusterModeTagStruct alloc] init];

    other.mfgCode = self.mfgCode;
    other.value = self.value;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: mfgCode:%@; value:%@; >", NSStringFromClass([self class]), _mfgCode, _value];
    return descriptionString;
}

@end

@implementation MTRWaterHeaterModeClusterModeOptionStruct
- (instancetype)init
{
    if (self = [super init]) {

        _label = @"";

        _mode = @(0);

        _modeTags = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRWaterHeaterModeClusterModeOptionStruct alloc] init];

    other.label = self.label;
    other.mode = self.mode;
    other.modeTags = self.modeTags;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: label:%@; mode:%@; modeTags:%@; >", NSStringFromClass([self class]), _label, _mode, _modeTags];
    return descriptionString;
}

@end

@implementation MTRDeviceEnergyManagementModeClusterModeTagStruct
- (instancetype)init
{
    if (self = [super init]) {

        _mfgCode = nil;

        _value = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDeviceEnergyManagementModeClusterModeTagStruct alloc] init];

    other.mfgCode = self.mfgCode;
    other.value = self.value;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: mfgCode:%@; value:%@; >", NSStringFromClass([self class]), _mfgCode, _value];
    return descriptionString;
}

@end

@implementation MTRDeviceEnergyManagementModeClusterModeOptionStruct
- (instancetype)init
{
    if (self = [super init]) {

        _label = @"";

        _mode = @(0);

        _modeTags = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRDeviceEnergyManagementModeClusterModeOptionStruct alloc] init];

    other.label = self.label;
    other.mode = self.mode;
    other.modeTags = self.modeTags;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: label:%@; mode:%@; modeTags:%@; >", NSStringFromClass([self class]), _label, _mode, _modeTags];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: credentialType:%@; credentialIndex:%@; >", NSStringFromClass([self class]), _credentialType, _credentialIndex];
    return descriptionString;
}

@end

@implementation MTRDoorLockClusterDlCredential : MTRDoorLockClusterCredentialStruct
@dynamic credentialType;
@dynamic credentialIndex;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: lockOperationType:%@; operationSource:%@; userIndex:%@; fabricIndex:%@; sourceNode:%@; credentials:%@; >", NSStringFromClass([self class]), _lockOperationType, _operationSource, _userIndex, _fabricIndex, _sourceNode, _credentials];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: lockOperationType:%@; operationSource:%@; operationError:%@; userIndex:%@; fabricIndex:%@; sourceNode:%@; credentials:%@; >", NSStringFromClass([self class]), _lockOperationType, _operationSource, _operationError, _userIndex, _fabricIndex, _sourceNode, _credentials];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: lockDataType:%@; dataOperationType:%@; operationSource:%@; userIndex:%@; fabricIndex:%@; sourceNode:%@; dataIndex:%@; >", NSStringFromClass([self class]), _lockDataType, _dataOperationType, _operationSource, _userIndex, _fabricIndex, _sourceNode, _dataIndex];
    return descriptionString;
}

@end

@implementation MTRClosureControlClusterOverallStateStruct
- (instancetype)init
{
    if (self = [super init]) {

        _positioning = nil;

        _latching = nil;

        _speed = nil;

        _extraInfo = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRClosureControlClusterOverallStateStruct alloc] init];

    other.positioning = self.positioning;
    other.latching = self.latching;
    other.speed = self.speed;
    other.extraInfo = self.extraInfo;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: positioning:%@; latching:%@; speed:%@; extraInfo:%@; >", NSStringFromClass([self class]), _positioning, _latching, _speed, _extraInfo];
    return descriptionString;
}

@end

@implementation MTRClosureControlClusterOverallTargetStruct
- (instancetype)init
{
    if (self = [super init]) {

        _tagPosition = nil;

        _tagLatch = nil;

        _speed = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRClosureControlClusterOverallTargetStruct alloc] init];

    other.tagPosition = self.tagPosition;
    other.tagLatch = self.tagLatch;
    other.speed = self.speed;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: tagPosition:%@; tagLatch:%@; speed:%@; >", NSStringFromClass([self class]), _tagPosition, _tagLatch, _speed];
    return descriptionString;
}

@end

@implementation MTRServiceAreaClusterLandmarkInfoStruct
- (instancetype)init
{
    if (self = [super init]) {

        _landmarkTag = @(0);

        _relativePositionTag = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRServiceAreaClusterLandmarkInfoStruct alloc] init];

    other.landmarkTag = self.landmarkTag;
    other.relativePositionTag = self.relativePositionTag;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: landmarkTag:%@; relativePositionTag:%@; >", NSStringFromClass([self class]), _landmarkTag, _relativePositionTag];
    return descriptionString;
}

@end

@implementation MTRServiceAreaClusterAreaInfoStruct
- (instancetype)init
{
    if (self = [super init]) {

        _locationInfo = nil;

        _landmarkInfo = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRServiceAreaClusterAreaInfoStruct alloc] init];

    other.locationInfo = self.locationInfo;
    other.landmarkInfo = self.landmarkInfo;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: locationInfo:%@; landmarkInfo:%@; >", NSStringFromClass([self class]), _locationInfo, _landmarkInfo];
    return descriptionString;
}

@end

@implementation MTRServiceAreaClusterAreaStruct
- (instancetype)init
{
    if (self = [super init]) {

        _areaID = @(0);

        _mapID = nil;

        _areaInfo = [MTRServiceAreaClusterAreaInfoStruct new];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRServiceAreaClusterAreaStruct alloc] init];

    other.areaID = self.areaID;
    other.mapID = self.mapID;
    other.areaInfo = self.areaInfo;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: areaID:%@; mapID:%@; areaInfo:%@; >", NSStringFromClass([self class]), _areaID, _mapID, _areaInfo];
    return descriptionString;
}

@end

@implementation MTRServiceAreaClusterMapStruct
- (instancetype)init
{
    if (self = [super init]) {

        _mapID = @(0);

        _name = @"";
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRServiceAreaClusterMapStruct alloc] init];

    other.mapID = self.mapID;
    other.name = self.name;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: mapID:%@; name:%@; >", NSStringFromClass([self class]), _mapID, _name];
    return descriptionString;
}

@end

@implementation MTRServiceAreaClusterProgressStruct
- (instancetype)init
{
    if (self = [super init]) {

        _areaID = @(0);

        _status = @(0);

        _totalOperationalTime = nil;

        _estimatedTime = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRServiceAreaClusterProgressStruct alloc] init];

    other.areaID = self.areaID;
    other.status = self.status;
    other.totalOperationalTime = self.totalOperationalTime;
    other.estimatedTime = self.estimatedTime;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: areaID:%@; status:%@; totalOperationalTime:%@; estimatedTime:%@; >", NSStringFromClass([self class]), _areaID, _status, _totalOperationalTime, _estimatedTime];
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

@implementation MTRThermostatClusterScheduleTransitionStruct
- (instancetype)init
{
    if (self = [super init]) {

        _dayOfWeek = @(0);

        _transitionTime = @(0);

        _presetHandle = nil;

        _systemMode = nil;

        _coolingSetpoint = nil;

        _heatingSetpoint = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRThermostatClusterScheduleTransitionStruct alloc] init];

    other.dayOfWeek = self.dayOfWeek;
    other.transitionTime = self.transitionTime;
    other.presetHandle = self.presetHandle;
    other.systemMode = self.systemMode;
    other.coolingSetpoint = self.coolingSetpoint;
    other.heatingSetpoint = self.heatingSetpoint;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: dayOfWeek:%@; transitionTime:%@; presetHandle:%@; systemMode:%@; coolingSetpoint:%@; heatingSetpoint:%@; >", NSStringFromClass([self class]), _dayOfWeek, _transitionTime, [_presetHandle base64EncodedStringWithOptions:0], _systemMode, _coolingSetpoint, _heatingSetpoint];
    return descriptionString;
}

@end

@implementation MTRThermostatClusterScheduleStruct
- (instancetype)init
{
    if (self = [super init]) {

        _scheduleHandle = nil;

        _systemMode = @(0);

        _name = nil;

        _presetHandle = nil;

        _transitions = [NSArray array];

        _builtIn = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRThermostatClusterScheduleStruct alloc] init];

    other.scheduleHandle = self.scheduleHandle;
    other.systemMode = self.systemMode;
    other.name = self.name;
    other.presetHandle = self.presetHandle;
    other.transitions = self.transitions;
    other.builtIn = self.builtIn;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: scheduleHandle:%@; systemMode:%@; name:%@; presetHandle:%@; transitions:%@; builtIn:%@; >", NSStringFromClass([self class]), [_scheduleHandle base64EncodedStringWithOptions:0], _systemMode, _name, [_presetHandle base64EncodedStringWithOptions:0], _transitions, _builtIn];
    return descriptionString;
}

@end

@implementation MTRThermostatClusterPresetStruct
- (instancetype)init
{
    if (self = [super init]) {

        _presetHandle = nil;

        _presetScenario = @(0);

        _name = nil;

        _coolingSetpoint = nil;

        _heatingSetpoint = nil;

        _builtIn = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRThermostatClusterPresetStruct alloc] init];

    other.presetHandle = self.presetHandle;
    other.presetScenario = self.presetScenario;
    other.name = self.name;
    other.coolingSetpoint = self.coolingSetpoint;
    other.heatingSetpoint = self.heatingSetpoint;
    other.builtIn = self.builtIn;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: presetHandle:%@; presetScenario:%@; name:%@; coolingSetpoint:%@; heatingSetpoint:%@; builtIn:%@; >", NSStringFromClass([self class]), [_presetHandle base64EncodedStringWithOptions:0], _presetScenario, _name, _coolingSetpoint, _heatingSetpoint, _builtIn];
    return descriptionString;
}

@end

@implementation MTRThermostatClusterPresetTypeStruct
- (instancetype)init
{
    if (self = [super init]) {

        _presetScenario = @(0);

        _numberOfPresets = @(0);

        _presetTypeFeatures = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRThermostatClusterPresetTypeStruct alloc] init];

    other.presetScenario = self.presetScenario;
    other.numberOfPresets = self.numberOfPresets;
    other.presetTypeFeatures = self.presetTypeFeatures;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: presetScenario:%@; numberOfPresets:%@; presetTypeFeatures:%@; >", NSStringFromClass([self class]), _presetScenario, _numberOfPresets, _presetTypeFeatures];
    return descriptionString;
}

@end

@implementation MTRThermostatClusterScheduleTypeStruct
- (instancetype)init
{
    if (self = [super init]) {

        _systemMode = @(0);

        _numberOfSchedules = @(0);

        _scheduleTypeFeatures = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRThermostatClusterScheduleTypeStruct alloc] init];

    other.systemMode = self.systemMode;
    other.numberOfSchedules = self.numberOfSchedules;
    other.scheduleTypeFeatures = self.scheduleTypeFeatures;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: systemMode:%@; numberOfSchedules:%@; scheduleTypeFeatures:%@; >", NSStringFromClass([self class]), _systemMode, _numberOfSchedules, _scheduleTypeFeatures];
    return descriptionString;
}

@end

@implementation MTRThermostatClusterWeeklyScheduleTransitionStruct
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
    auto other = [[MTRThermostatClusterWeeklyScheduleTransitionStruct alloc] init];

    other.transitionTime = self.transitionTime;
    other.heatSetpoint = self.heatSetpoint;
    other.coolSetpoint = self.coolSetpoint;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: transitionTime:%@; heatSetpoint:%@; coolSetpoint:%@; >", NSStringFromClass([self class]), _transitionTime, _heatSetpoint, _coolSetpoint];
    return descriptionString;
}

@end

@implementation MTRThermostatClusterThermostatScheduleTransition : MTRThermostatClusterWeeklyScheduleTransitionStruct
@dynamic transitionTime;
@dynamic heatSetpoint;
@dynamic coolSetpoint;
@end

@implementation MTROccupancySensingClusterHoldTimeLimitsStruct
- (instancetype)init
{
    if (self = [super init]) {

        _holdTimeMin = @(0);

        _holdTimeMax = @(0);

        _holdTimeDefault = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTROccupancySensingClusterHoldTimeLimitsStruct alloc] init];

    other.holdTimeMin = self.holdTimeMin;
    other.holdTimeMax = self.holdTimeMax;
    other.holdTimeDefault = self.holdTimeDefault;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: holdTimeMin:%@; holdTimeMax:%@; holdTimeDefault:%@; >", NSStringFromClass([self class]), _holdTimeMin, _holdTimeMax, _holdTimeDefault];
    return descriptionString;
}

@end

@implementation MTROccupancySensingClusterOccupancyChangedEvent
- (instancetype)init
{
    if (self = [super init]) {

        _occupancy = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTROccupancySensingClusterOccupancyChangedEvent alloc] init];

    other.occupancy = self.occupancy;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: occupancy:%@; >", NSStringFromClass([self class]), _occupancy];
    return descriptionString;
}

@end

@implementation MTRThreadNetworkDirectoryClusterThreadNetworkStruct
- (instancetype)init
{
    if (self = [super init]) {

        _extendedPanID = [NSData data];

        _networkName = @"";

        _channel = @(0);

        _activeTimestamp = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRThreadNetworkDirectoryClusterThreadNetworkStruct alloc] init];

    other.extendedPanID = self.extendedPanID;
    other.networkName = self.networkName;
    other.channel = self.channel;
    other.activeTimestamp = self.activeTimestamp;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: extendedPanID:%@; networkName:%@; channel:%@; activeTimestamp:%@; >", NSStringFromClass([self class]), [_extendedPanID base64EncodedStringWithOptions:0], _networkName, _channel, _activeTimestamp];
    return descriptionString;
}

@end

@implementation MTRChannelClusterProgramCastStruct
- (instancetype)init
{
    if (self = [super init]) {

        _name = @"";

        _role = @"";
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRChannelClusterProgramCastStruct alloc] init];

    other.name = self.name;
    other.role = self.role;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: name:%@; role:%@; >", NSStringFromClass([self class]), _name, _role];
    return descriptionString;
}

@end

@implementation MTRChannelClusterProgramCategoryStruct
- (instancetype)init
{
    if (self = [super init]) {

        _category = @"";

        _subCategory = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRChannelClusterProgramCategoryStruct alloc] init];

    other.category = self.category;
    other.subCategory = self.subCategory;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: category:%@; subCategory:%@; >", NSStringFromClass([self class]), _category, _subCategory];
    return descriptionString;
}

@end

@implementation MTRChannelClusterSeriesInfoStruct
- (instancetype)init
{
    if (self = [super init]) {

        _season = @"";

        _episode = @"";
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRChannelClusterSeriesInfoStruct alloc] init];

    other.season = self.season;
    other.episode = self.episode;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: season:%@; episode:%@; >", NSStringFromClass([self class]), _season, _episode];
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

        _identifier = nil;

        _type = nil;
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
    other.identifier = self.identifier;
    other.type = self.type;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: majorNumber:%@; minorNumber:%@; name:%@; callSign:%@; affiliateCallSign:%@; identifier:%@; type:%@; >", NSStringFromClass([self class]), _majorNumber, _minorNumber, _name, _callSign, _affiliateCallSign, _identifier, _type];
    return descriptionString;
}

@end

@implementation MTRChannelClusterChannelInfo : MTRChannelClusterChannelInfoStruct
@dynamic majorNumber;
@dynamic minorNumber;
@dynamic name;
@dynamic callSign;
@dynamic affiliateCallSign;
@end

@implementation MTRChannelClusterProgramStruct
- (instancetype)init
{
    if (self = [super init]) {

        _identifier = @"";

        _channel = [MTRChannelClusterChannelInfoStruct new];

        _startTime = @(0);

        _endTime = @(0);

        _title = @"";

        _subtitle = nil;

        _descriptionString = nil;

        _audioLanguages = nil;

        _ratings = nil;

        _thumbnailUrl = nil;

        _posterArtUrl = nil;

        _dvbiUrl = nil;

        _releaseDate = nil;

        _parentalGuidanceText = nil;

        _recordingFlag = nil;

        _seriesInfo = nil;

        _categoryList = nil;

        _castList = nil;

        _externalIDList = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRChannelClusterProgramStruct alloc] init];

    other.identifier = self.identifier;
    other.channel = self.channel;
    other.startTime = self.startTime;
    other.endTime = self.endTime;
    other.title = self.title;
    other.subtitle = self.subtitle;
    other.descriptionString = self.descriptionString;
    other.audioLanguages = self.audioLanguages;
    other.ratings = self.ratings;
    other.thumbnailUrl = self.thumbnailUrl;
    other.posterArtUrl = self.posterArtUrl;
    other.dvbiUrl = self.dvbiUrl;
    other.releaseDate = self.releaseDate;
    other.parentalGuidanceText = self.parentalGuidanceText;
    other.recordingFlag = self.recordingFlag;
    other.seriesInfo = self.seriesInfo;
    other.categoryList = self.categoryList;
    other.castList = self.castList;
    other.externalIDList = self.externalIDList;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: identifier:%@; channel:%@; startTime:%@; endTime:%@; title:%@; subtitle:%@; descriptionString:%@; audioLanguages:%@; ratings:%@; thumbnailUrl:%@; posterArtUrl:%@; dvbiUrl:%@; releaseDate:%@; parentalGuidanceText:%@; recordingFlag:%@; seriesInfo:%@; categoryList:%@; castList:%@; externalIDList:%@; >", NSStringFromClass([self class]), _identifier, _channel, _startTime, _endTime, _title, _subtitle, _descriptionString, _audioLanguages, _ratings, _thumbnailUrl, _posterArtUrl, _dvbiUrl, _releaseDate, _parentalGuidanceText, _recordingFlag, _seriesInfo, _categoryList, _castList, _externalIDList];
    return descriptionString;
}

@end

@implementation MTRChannelClusterPageTokenStruct
- (instancetype)init
{
    if (self = [super init]) {

        _limit = nil;

        _after = nil;

        _before = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRChannelClusterPageTokenStruct alloc] init];

    other.limit = self.limit;
    other.after = self.after;
    other.before = self.before;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: limit:%@; after:%@; before:%@; >", NSStringFromClass([self class]), _limit, _after, _before];
    return descriptionString;
}

@end

@implementation MTRChannelClusterChannelPagingStruct
- (instancetype)init
{
    if (self = [super init]) {

        _previousToken = nil;

        _nextToken = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRChannelClusterChannelPagingStruct alloc] init];

    other.previousToken = self.previousToken;
    other.nextToken = self.nextToken;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: previousToken:%@; nextToken:%@; >", NSStringFromClass([self class]), _previousToken, _nextToken];
    return descriptionString;
}

@end

@implementation MTRChannelClusterAdditionalInfoStruct
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
    auto other = [[MTRChannelClusterAdditionalInfoStruct alloc] init];

    other.name = self.name;
    other.value = self.value;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: name:%@; value:%@; >", NSStringFromClass([self class]), _name, _value];
    return descriptionString;
}

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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: operatorName:%@; lineupName:%@; postalCode:%@; lineupInfoType:%@; >", NSStringFromClass([self class]), _operatorName, _lineupName, _postalCode, _lineupInfoType];
    return descriptionString;
}

@end

@implementation MTRChannelClusterLineupInfo : MTRChannelClusterLineupInfoStruct
@dynamic operatorName;
@dynamic lineupName;
@dynamic postalCode;
@dynamic lineupInfoType;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: identifier:%@; name:%@; >", NSStringFromClass([self class]), _identifier, _name];
    return descriptionString;
}

@end

@implementation MTRTargetNavigatorClusterTargetInfo : MTRTargetNavigatorClusterTargetInfoStruct
@dynamic identifier;
@dynamic name;
@end

@implementation MTRTargetNavigatorClusterTargetUpdatedEvent
- (instancetype)init
{
    if (self = [super init]) {

        _targetList = [NSArray array];

        _currentTarget = @(0);

        _data = [NSData data];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRTargetNavigatorClusterTargetUpdatedEvent alloc] init];

    other.targetList = self.targetList;
    other.currentTarget = self.currentTarget;
    other.data = self.data;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: targetList:%@; currentTarget:%@; data:%@; >", NSStringFromClass([self class]), _targetList, _currentTarget, [_data base64EncodedStringWithOptions:0]];
    return descriptionString;
}

@end

@implementation MTRMediaPlaybackClusterTrackAttributesStruct
- (instancetype)init
{
    if (self = [super init]) {

        _languageCode = @"";

        _displayName = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRMediaPlaybackClusterTrackAttributesStruct alloc] init];

    other.languageCode = self.languageCode;
    other.displayName = self.displayName;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: languageCode:%@; displayName:%@; >", NSStringFromClass([self class]), _languageCode, _displayName];
    return descriptionString;
}

@end

@implementation MTRMediaPlaybackClusterTrackStruct
- (instancetype)init
{
    if (self = [super init]) {

        _id = @"";

        _trackAttributes = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRMediaPlaybackClusterTrackStruct alloc] init];

    other.id = self.id;
    other.trackAttributes = self.trackAttributes;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: id:%@; trackAttributes:%@; >", NSStringFromClass([self class]), _id, _trackAttributes];
    return descriptionString;
}

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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: updatedAt:%@; position:%@; >", NSStringFromClass([self class]), _updatedAt, _position];
    return descriptionString;
}

@end

@implementation MTRMediaPlaybackClusterPlaybackPosition : MTRMediaPlaybackClusterPlaybackPositionStruct
@dynamic updatedAt;
@dynamic position;
@end

@implementation MTRMediaPlaybackClusterStateChangedEvent
- (instancetype)init
{
    if (self = [super init]) {

        _currentState = @(0);

        _startTime = @(0);

        _duration = @(0);

        _sampledPosition = [MTRMediaPlaybackClusterPlaybackPositionStruct new];

        _playbackSpeed = @(0);

        _seekRangeEnd = @(0);

        _seekRangeStart = @(0);

        _data = nil;

        _audioAdvanceUnmuted = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRMediaPlaybackClusterStateChangedEvent alloc] init];

    other.currentState = self.currentState;
    other.startTime = self.startTime;
    other.duration = self.duration;
    other.sampledPosition = self.sampledPosition;
    other.playbackSpeed = self.playbackSpeed;
    other.seekRangeEnd = self.seekRangeEnd;
    other.seekRangeStart = self.seekRangeStart;
    other.data = self.data;
    other.audioAdvanceUnmuted = self.audioAdvanceUnmuted;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: currentState:%@; startTime:%@; duration:%@; sampledPosition:%@; playbackSpeed:%@; seekRangeEnd:%@; seekRangeStart:%@; data:%@; audioAdvanceUnmuted:%@; >", NSStringFromClass([self class]), _currentState, _startTime, _duration, _sampledPosition, _playbackSpeed, _seekRangeEnd, _seekRangeStart, [_data base64EncodedStringWithOptions:0], _audioAdvanceUnmuted];
    return descriptionString;
}

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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: index:%@; inputType:%@; name:%@; descriptionString:%@; >", NSStringFromClass([self class]), _index, _inputType, _name, _descriptionString];
    return descriptionString;
}

@end

@implementation MTRMediaInputClusterInputInfo : MTRMediaInputClusterInputInfoStruct
@dynamic index;
@dynamic inputType;
@dynamic name;
@dynamic descriptionString;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: width:%@; height:%@; metric:%@; >", NSStringFromClass([self class]), _width, _height, _metric];
    return descriptionString;
}

@end

@implementation MTRContentLauncherClusterDimension : MTRContentLauncherClusterDimensionStruct
@dynamic width;
@dynamic height;
@dynamic metric;
@end

@implementation MTRContentLauncherClusterTrackPreferenceStruct
- (instancetype)init
{
    if (self = [super init]) {

        _languageCode = @"";

        _characteristics = nil;

        _audioOutputIndex = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRContentLauncherClusterTrackPreferenceStruct alloc] init];

    other.languageCode = self.languageCode;
    other.characteristics = self.characteristics;
    other.audioOutputIndex = self.audioOutputIndex;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: languageCode:%@; characteristics:%@; audioOutputIndex:%@; >", NSStringFromClass([self class]), _languageCode, _characteristics, _audioOutputIndex];
    return descriptionString;
}

@end

@implementation MTRContentLauncherClusterPlaybackPreferencesStruct
- (instancetype)init
{
    if (self = [super init]) {

        _playbackPosition = @(0);

        _textTrack = [MTRContentLauncherClusterTrackPreferenceStruct new];

        _audioTracks = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRContentLauncherClusterPlaybackPreferencesStruct alloc] init];

    other.playbackPosition = self.playbackPosition;
    other.textTrack = self.textTrack;
    other.audioTracks = self.audioTracks;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: playbackPosition:%@; textTrack:%@; audioTracks:%@; >", NSStringFromClass([self class]), _playbackPosition, _textTrack, _audioTracks];
    return descriptionString;
}

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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: name:%@; value:%@; >", NSStringFromClass([self class]), _name, _value];
    return descriptionString;
}

@end

@implementation MTRContentLauncherClusterAdditionalInfo : MTRContentLauncherClusterAdditionalInfoStruct
@dynamic name;
@dynamic value;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: type:%@; value:%@; externalIDList:%@; >", NSStringFromClass([self class]), _type, _value, _externalIDList];
    return descriptionString;
}

@end

@implementation MTRContentLauncherClusterParameter : MTRContentLauncherClusterParameterStruct
@dynamic type;
@dynamic value;
@dynamic externalIDList;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: parameterList:%@; >", NSStringFromClass([self class]), _parameterList];
    return descriptionString;
}

@end

@implementation MTRContentLauncherClusterContentSearch : MTRContentLauncherClusterContentSearchStruct
@dynamic parameterList;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: imageURL:%@; color:%@; size:%@; >", NSStringFromClass([self class]), _imageURL, _color, _size];
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
@dynamic color;
@dynamic size;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: providerName:%@; background:%@; logo:%@; progressBar:%@; splash:%@; waterMark:%@; >", NSStringFromClass([self class]), _providerName, _background, _logo, _progressBar, _splash, _waterMark];
    return descriptionString;
}

@end

@implementation MTRContentLauncherClusterBrandingInformation : MTRContentLauncherClusterBrandingInformationStruct
@dynamic providerName;
@dynamic background;
@dynamic logo;
@dynamic progressBar;
@dynamic splash;
@dynamic waterMark;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: index:%@; outputType:%@; name:%@; >", NSStringFromClass([self class]), _index, _outputType, _name];
    return descriptionString;
}

@end

@implementation MTRAudioOutputClusterOutputInfo : MTRAudioOutputClusterOutputInfoStruct
@dynamic index;
@dynamic outputType;
@dynamic name;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: catalogVendorID:%@; applicationID:%@; >", NSStringFromClass([self class]), _catalogVendorID, _applicationID];
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: application:%@; endpoint:%@; >", NSStringFromClass([self class]), _application, _endpoint];
    return descriptionString;
}

@end

@implementation MTRApplicationLauncherClusterApplicationEP : MTRApplicationLauncherClusterApplicationEPStruct
@dynamic application;
@dynamic endpoint;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: catalogVendorID:%@; applicationID:%@; >", NSStringFromClass([self class]), _catalogVendorID, _applicationID];
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

@implementation MTRAccountLoginClusterLoggedOutEvent
- (instancetype)init
{
    if (self = [super init]) {

        _node = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRAccountLoginClusterLoggedOutEvent alloc] init];

    other.node = self.node;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: node:%@; >", NSStringFromClass([self class]), _node];
    return descriptionString;
}

@end

@implementation MTRContentControlClusterRatingNameStruct
- (instancetype)init
{
    if (self = [super init]) {

        _ratingName = @"";

        _ratingNameDesc = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRContentControlClusterRatingNameStruct alloc] init];

    other.ratingName = self.ratingName;
    other.ratingNameDesc = self.ratingNameDesc;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: ratingName:%@; ratingNameDesc:%@; >", NSStringFromClass([self class]), _ratingName, _ratingNameDesc];
    return descriptionString;
}

@end

@implementation MTRContentControlClusterRemainingScreenTimeExpiredEvent
- (instancetype)init
{
    if (self = [super init]) {
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRContentControlClusterRemainingScreenTimeExpiredEvent alloc] init];

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRZoneManagementClusterTwoDCartesianVertexStruct
- (instancetype)init
{
    if (self = [super init]) {

        _x = @(0);

        _y = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRZoneManagementClusterTwoDCartesianVertexStruct alloc] init];

    other.x = self.x;
    other.y = self.y;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: x:%@; y:%@; >", NSStringFromClass([self class]), _x, _y];
    return descriptionString;
}

@end

@implementation MTRZoneManagementClusterTwoDCartesianZoneStruct
- (instancetype)init
{
    if (self = [super init]) {

        _name = @"";

        _use = @(0);

        _vertices = [NSArray array];

        _color = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRZoneManagementClusterTwoDCartesianZoneStruct alloc] init];

    other.name = self.name;
    other.use = self.use;
    other.vertices = self.vertices;
    other.color = self.color;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: name:%@; use:%@; vertices:%@; color:%@; >", NSStringFromClass([self class]), _name, _use, _vertices, _color];
    return descriptionString;
}

@end

@implementation MTRZoneManagementClusterZoneInformationStruct
- (instancetype)init
{
    if (self = [super init]) {

        _zoneID = @(0);

        _zoneType = @(0);

        _zoneSource = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRZoneManagementClusterZoneInformationStruct alloc] init];

    other.zoneID = self.zoneID;
    other.zoneType = self.zoneType;
    other.zoneSource = self.zoneSource;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: zoneID:%@; zoneType:%@; zoneSource:%@; >", NSStringFromClass([self class]), _zoneID, _zoneType, _zoneSource];
    return descriptionString;
}

@end

@implementation MTRZoneManagementClusterZoneTriggerControlStruct
- (instancetype)init
{
    if (self = [super init]) {

        _initialDuration = @(0);

        _augmentationDuration = @(0);

        _maxDuration = @(0);

        _blindDuration = @(0);

        _sensitivity = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRZoneManagementClusterZoneTriggerControlStruct alloc] init];

    other.initialDuration = self.initialDuration;
    other.augmentationDuration = self.augmentationDuration;
    other.maxDuration = self.maxDuration;
    other.blindDuration = self.blindDuration;
    other.sensitivity = self.sensitivity;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: initialDuration:%@; augmentationDuration:%@; maxDuration:%@; blindDuration:%@; sensitivity:%@; >", NSStringFromClass([self class]), _initialDuration, _augmentationDuration, _maxDuration, _blindDuration, _sensitivity];
    return descriptionString;
}

@end

@implementation MTRZoneManagementClusterZoneTriggeredEvent
- (instancetype)init
{
    if (self = [super init]) {

        _zones = [NSArray array];

        _reason = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRZoneManagementClusterZoneTriggeredEvent alloc] init];

    other.zones = self.zones;
    other.reason = self.reason;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: zones:%@; reason:%@; >", NSStringFromClass([self class]), _zones, _reason];
    return descriptionString;
}

@end

@implementation MTRZoneManagementClusterZoneStoppedEvent
- (instancetype)init
{
    if (self = [super init]) {

        _zones = [NSArray array];

        _reason = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRZoneManagementClusterZoneStoppedEvent alloc] init];

    other.zones = self.zones;
    other.reason = self.reason;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: zones:%@; reason:%@; >", NSStringFromClass([self class]), _zones, _reason];
    return descriptionString;
}

@end

@implementation MTRCameraAVStreamManagementClusterVideoResolutionStruct
- (instancetype)init
{
    if (self = [super init]) {

        _width = @(0);

        _height = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRCameraAVStreamManagementClusterVideoResolutionStruct alloc] init];

    other.width = self.width;
    other.height = self.height;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: width:%@; height:%@; >", NSStringFromClass([self class]), _width, _height];
    return descriptionString;
}

@end

@implementation MTRCameraAVStreamManagementClusterVideoStreamStruct
- (instancetype)init
{
    if (self = [super init]) {

        _videoStreamID = @(0);

        _streamUsage = @(0);

        _videoCodec = @(0);

        _minFrameRate = @(0);

        _maxFrameRate = @(0);

        _minResolution = [MTRCameraAVStreamManagementClusterVideoResolutionStruct new];

        _maxResolution = [MTRCameraAVStreamManagementClusterVideoResolutionStruct new];

        _minBitRate = @(0);

        _maxBitRate = @(0);

        _minFragmentLen = @(0);

        _maxFragmentLen = @(0);

        _watermarkEnabled = nil;

        _osdEnabled = nil;

        _referenceCount = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRCameraAVStreamManagementClusterVideoStreamStruct alloc] init];

    other.videoStreamID = self.videoStreamID;
    other.streamUsage = self.streamUsage;
    other.videoCodec = self.videoCodec;
    other.minFrameRate = self.minFrameRate;
    other.maxFrameRate = self.maxFrameRate;
    other.minResolution = self.minResolution;
    other.maxResolution = self.maxResolution;
    other.minBitRate = self.minBitRate;
    other.maxBitRate = self.maxBitRate;
    other.minFragmentLen = self.minFragmentLen;
    other.maxFragmentLen = self.maxFragmentLen;
    other.watermarkEnabled = self.watermarkEnabled;
    other.osdEnabled = self.osdEnabled;
    other.referenceCount = self.referenceCount;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: videoStreamID:%@; streamUsage:%@; videoCodec:%@; minFrameRate:%@; maxFrameRate:%@; minResolution:%@; maxResolution:%@; minBitRate:%@; maxBitRate:%@; minFragmentLen:%@; maxFragmentLen:%@; watermarkEnabled:%@; osdEnabled:%@; referenceCount:%@; >", NSStringFromClass([self class]), _videoStreamID, _streamUsage, _videoCodec, _minFrameRate, _maxFrameRate, _minResolution, _maxResolution, _minBitRate, _maxBitRate, _minFragmentLen, _maxFragmentLen, _watermarkEnabled, _osdEnabled, _referenceCount];
    return descriptionString;
}

@end

@implementation MTRCameraAVStreamManagementClusterSnapshotStreamStruct
- (instancetype)init
{
    if (self = [super init]) {

        _snapshotStreamID = @(0);

        _imageCodec = @(0);

        _frameRate = @(0);

        _bitRate = @(0);

        _minResolution = [MTRCameraAVStreamManagementClusterVideoResolutionStruct new];

        _maxResolution = [MTRCameraAVStreamManagementClusterVideoResolutionStruct new];

        _quality = @(0);

        _referenceCount = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRCameraAVStreamManagementClusterSnapshotStreamStruct alloc] init];

    other.snapshotStreamID = self.snapshotStreamID;
    other.imageCodec = self.imageCodec;
    other.frameRate = self.frameRate;
    other.bitRate = self.bitRate;
    other.minResolution = self.minResolution;
    other.maxResolution = self.maxResolution;
    other.quality = self.quality;
    other.referenceCount = self.referenceCount;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: snapshotStreamID:%@; imageCodec:%@; frameRate:%@; bitRate:%@; minResolution:%@; maxResolution:%@; quality:%@; referenceCount:%@; >", NSStringFromClass([self class]), _snapshotStreamID, _imageCodec, _frameRate, _bitRate, _minResolution, _maxResolution, _quality, _referenceCount];
    return descriptionString;
}

@end

@implementation MTRCameraAVStreamManagementClusterSnapshotParamsStruct
- (instancetype)init
{
    if (self = [super init]) {

        _resolution = [MTRCameraAVStreamManagementClusterVideoResolutionStruct new];

        _maxFrameRate = @(0);

        _imageCodec = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRCameraAVStreamManagementClusterSnapshotParamsStruct alloc] init];

    other.resolution = self.resolution;
    other.maxFrameRate = self.maxFrameRate;
    other.imageCodec = self.imageCodec;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: resolution:%@; maxFrameRate:%@; imageCodec:%@; >", NSStringFromClass([self class]), _resolution, _maxFrameRate, _imageCodec];
    return descriptionString;
}

@end

@implementation MTRCameraAVStreamManagementClusterRateDistortionTradeOffPointsStruct
- (instancetype)init
{
    if (self = [super init]) {

        _codec = @(0);

        _resolution = [MTRCameraAVStreamManagementClusterVideoResolutionStruct new];

        _minBitRate = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRCameraAVStreamManagementClusterRateDistortionTradeOffPointsStruct alloc] init];

    other.codec = self.codec;
    other.resolution = self.resolution;
    other.minBitRate = self.minBitRate;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: codec:%@; resolution:%@; minBitRate:%@; >", NSStringFromClass([self class]), _codec, _resolution, _minBitRate];
    return descriptionString;
}

@end

@implementation MTRCameraAVStreamManagementClusterAudioCapabilitiesStruct
- (instancetype)init
{
    if (self = [super init]) {

        _maxNumberOfChannels = @(0);

        _supportedCodecs = [NSArray array];

        _supportedSampleRates = [NSArray array];

        _supportedBitDepths = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRCameraAVStreamManagementClusterAudioCapabilitiesStruct alloc] init];

    other.maxNumberOfChannels = self.maxNumberOfChannels;
    other.supportedCodecs = self.supportedCodecs;
    other.supportedSampleRates = self.supportedSampleRates;
    other.supportedBitDepths = self.supportedBitDepths;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: maxNumberOfChannels:%@; supportedCodecs:%@; supportedSampleRates:%@; supportedBitDepths:%@; >", NSStringFromClass([self class]), _maxNumberOfChannels, _supportedCodecs, _supportedSampleRates, _supportedBitDepths];
    return descriptionString;
}

@end

@implementation MTRCameraAVStreamManagementClusterAudioStreamStruct
- (instancetype)init
{
    if (self = [super init]) {

        _audioStreamID = @(0);

        _streamUsage = @(0);

        _audioCodec = @(0);

        _channelCount = @(0);

        _sampleRate = @(0);

        _bitRate = @(0);

        _bitDepth = @(0);

        _referenceCount = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRCameraAVStreamManagementClusterAudioStreamStruct alloc] init];

    other.audioStreamID = self.audioStreamID;
    other.streamUsage = self.streamUsage;
    other.audioCodec = self.audioCodec;
    other.channelCount = self.channelCount;
    other.sampleRate = self.sampleRate;
    other.bitRate = self.bitRate;
    other.bitDepth = self.bitDepth;
    other.referenceCount = self.referenceCount;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: audioStreamID:%@; streamUsage:%@; audioCodec:%@; channelCount:%@; sampleRate:%@; bitRate:%@; bitDepth:%@; referenceCount:%@; >", NSStringFromClass([self class]), _audioStreamID, _streamUsage, _audioCodec, _channelCount, _sampleRate, _bitRate, _bitDepth, _referenceCount];
    return descriptionString;
}

@end

@implementation MTRCameraAVStreamManagementClusterVideoSensorParamsStruct
- (instancetype)init
{
    if (self = [super init]) {

        _sensorWidth = @(0);

        _sensorHeight = @(0);

        _maxFPS = @(0);

        _maxHDRFPS = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRCameraAVStreamManagementClusterVideoSensorParamsStruct alloc] init];

    other.sensorWidth = self.sensorWidth;
    other.sensorHeight = self.sensorHeight;
    other.maxFPS = self.maxFPS;
    other.maxHDRFPS = self.maxHDRFPS;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: sensorWidth:%@; sensorHeight:%@; maxFPS:%@; maxHDRFPS:%@; >", NSStringFromClass([self class]), _sensorWidth, _sensorHeight, _maxFPS, _maxHDRFPS];
    return descriptionString;
}

@end

@implementation MTRCameraAVStreamManagementClusterViewportStruct
- (instancetype)init
{
    if (self = [super init]) {

        _x1 = @(0);

        _y1 = @(0);

        _x2 = @(0);

        _y2 = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRCameraAVStreamManagementClusterViewportStruct alloc] init];

    other.x1 = self.x1;
    other.y1 = self.y1;
    other.x2 = self.x2;
    other.y2 = self.y2;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: x1:%@; y1:%@; x2:%@; y2:%@; >", NSStringFromClass([self class]), _x1, _y1, _x2, _y2];
    return descriptionString;
}

@end

@implementation MTRCameraAVSettingsUserLevelManagementClusterMPTZStruct
- (instancetype)init
{
    if (self = [super init]) {

        _pan = nil;

        _tilt = nil;

        _zoom = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRCameraAVSettingsUserLevelManagementClusterMPTZStruct alloc] init];

    other.pan = self.pan;
    other.tilt = self.tilt;
    other.zoom = self.zoom;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: pan:%@; tilt:%@; zoom:%@; >", NSStringFromClass([self class]), _pan, _tilt, _zoom];
    return descriptionString;
}

@end

@implementation MTRCameraAVSettingsUserLevelManagementClusterMPTZPresetStruct
- (instancetype)init
{
    if (self = [super init]) {

        _presetID = @(0);

        _name = @"";

        _settings = [MTRCameraAVSettingsUserLevelManagementClusterMPTZStruct new];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRCameraAVSettingsUserLevelManagementClusterMPTZPresetStruct alloc] init];

    other.presetID = self.presetID;
    other.name = self.name;
    other.settings = self.settings;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: presetID:%@; name:%@; settings:%@; >", NSStringFromClass([self class]), _presetID, _name, _settings];
    return descriptionString;
}

@end

@implementation MTRCameraAVSettingsUserLevelManagementClusterViewportStruct
- (instancetype)init
{
    if (self = [super init]) {

        _x1 = @(0);

        _y1 = @(0);

        _x2 = @(0);

        _y2 = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRCameraAVSettingsUserLevelManagementClusterViewportStruct alloc] init];

    other.x1 = self.x1;
    other.y1 = self.y1;
    other.x2 = self.x2;
    other.y2 = self.y2;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: x1:%@; y1:%@; x2:%@; y2:%@; >", NSStringFromClass([self class]), _x1, _y1, _x2, _y2];
    return descriptionString;
}

@end

@implementation MTRWebRTCTransportProviderClusterICEServerStruct
- (instancetype)init
{
    if (self = [super init]) {

        _urls = [NSArray array];

        _username = nil;

        _credential = nil;

        _caid = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRWebRTCTransportProviderClusterICEServerStruct alloc] init];

    other.urls = self.urls;
    other.username = self.username;
    other.credential = self.credential;
    other.caid = self.caid;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: urls:%@; username:%@; credential:%@; caid:%@; >", NSStringFromClass([self class]), _urls, _username, _credential, _caid];
    return descriptionString;
}

@end

@implementation MTRWebRTCTransportProviderClusterWebRTCSessionStruct
- (instancetype)init
{
    if (self = [super init]) {

        _id = @(0);

        _peerNodeID = @(0);

        _streamUsage = @(0);

        _videoStreamID = nil;

        _audioStreamID = nil;

        _metadataOptions = @(0);

        _fabricIndex = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRWebRTCTransportProviderClusterWebRTCSessionStruct alloc] init];

    other.id = self.id;
    other.peerNodeID = self.peerNodeID;
    other.streamUsage = self.streamUsage;
    other.videoStreamID = self.videoStreamID;
    other.audioStreamID = self.audioStreamID;
    other.metadataOptions = self.metadataOptions;
    other.fabricIndex = self.fabricIndex;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: id:%@; peerNodeID:%@; streamUsage:%@; videoStreamID:%@; audioStreamID:%@; metadataOptions:%@; fabricIndex:%@; >", NSStringFromClass([self class]), _id, _peerNodeID, _streamUsage, _videoStreamID, _audioStreamID, _metadataOptions, _fabricIndex];
    return descriptionString;
}

@end

@implementation MTRWebRTCTransportRequestorClusterICEServerStruct
- (instancetype)init
{
    if (self = [super init]) {

        _urls = [NSArray array];

        _username = nil;

        _credential = nil;

        _caid = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRWebRTCTransportRequestorClusterICEServerStruct alloc] init];

    other.urls = self.urls;
    other.username = self.username;
    other.credential = self.credential;
    other.caid = self.caid;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: urls:%@; username:%@; credential:%@; caid:%@; >", NSStringFromClass([self class]), _urls, _username, _credential, _caid];
    return descriptionString;
}

@end

@implementation MTRWebRTCTransportRequestorClusterWebRTCSessionStruct
- (instancetype)init
{
    if (self = [super init]) {

        _id = @(0);

        _peerNodeID = @(0);

        _streamUsage = @(0);

        _videoStreamID = nil;

        _audioStreamID = nil;

        _metadataOptions = @(0);

        _fabricIndex = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRWebRTCTransportRequestorClusterWebRTCSessionStruct alloc] init];

    other.id = self.id;
    other.peerNodeID = self.peerNodeID;
    other.streamUsage = self.streamUsage;
    other.videoStreamID = self.videoStreamID;
    other.audioStreamID = self.audioStreamID;
    other.metadataOptions = self.metadataOptions;
    other.fabricIndex = self.fabricIndex;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: id:%@; peerNodeID:%@; streamUsage:%@; videoStreamID:%@; audioStreamID:%@; metadataOptions:%@; fabricIndex:%@; >", NSStringFromClass([self class]), _id, _peerNodeID, _streamUsage, _videoStreamID, _audioStreamID, _metadataOptions, _fabricIndex];
    return descriptionString;
}

@end

@implementation MTRPushAVStreamTransportClusterTransportMotionTriggerTimeControlStruct
- (instancetype)init
{
    if (self = [super init]) {

        _initialDuration = @(0);

        _augmentationDuration = @(0);

        _maxDuration = @(0);

        _blindDuration = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPushAVStreamTransportClusterTransportMotionTriggerTimeControlStruct alloc] init];

    other.initialDuration = self.initialDuration;
    other.augmentationDuration = self.augmentationDuration;
    other.maxDuration = self.maxDuration;
    other.blindDuration = self.blindDuration;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: initialDuration:%@; augmentationDuration:%@; maxDuration:%@; blindDuration:%@; >", NSStringFromClass([self class]), _initialDuration, _augmentationDuration, _maxDuration, _blindDuration];
    return descriptionString;
}

@end

@implementation MTRPushAVStreamTransportClusterTransportZoneOptionsStruct
- (instancetype)init
{
    if (self = [super init]) {

        _zone = nil;

        _sensitivity = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPushAVStreamTransportClusterTransportZoneOptionsStruct alloc] init];

    other.zone = self.zone;
    other.sensitivity = self.sensitivity;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: zone:%@; sensitivity:%@; >", NSStringFromClass([self class]), _zone, _sensitivity];
    return descriptionString;
}

@end

@implementation MTRPushAVStreamTransportClusterMetadataOptionsStruct
- (instancetype)init
{
    if (self = [super init]) {

        _multiplexing = @(0);

        _includeMotionZones = @(0);

        _enableMetadataPrivacySensitive = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPushAVStreamTransportClusterMetadataOptionsStruct alloc] init];

    other.multiplexing = self.multiplexing;
    other.includeMotionZones = self.includeMotionZones;
    other.enableMetadataPrivacySensitive = self.enableMetadataPrivacySensitive;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: multiplexing:%@; includeMotionZones:%@; enableMetadataPrivacySensitive:%@; >", NSStringFromClass([self class]), _multiplexing, _includeMotionZones, _enableMetadataPrivacySensitive];
    return descriptionString;
}

@end

@implementation MTRPushAVStreamTransportClusterTransportTriggerOptionsStruct
- (instancetype)init
{
    if (self = [super init]) {

        _triggerType = @(0);

        _motionZones = nil;

        _motionSensitivity = nil;

        _motionTimeControl = nil;

        _maxPreRollLen = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPushAVStreamTransportClusterTransportTriggerOptionsStruct alloc] init];

    other.triggerType = self.triggerType;
    other.motionZones = self.motionZones;
    other.motionSensitivity = self.motionSensitivity;
    other.motionTimeControl = self.motionTimeControl;
    other.maxPreRollLen = self.maxPreRollLen;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: triggerType:%@; motionZones:%@; motionSensitivity:%@; motionTimeControl:%@; maxPreRollLen:%@; >", NSStringFromClass([self class]), _triggerType, _motionZones, _motionSensitivity, _motionTimeControl, _maxPreRollLen];
    return descriptionString;
}

@end

@implementation MTRPushAVStreamTransportClusterCMAFContainerOptionsStruct
- (instancetype)init
{
    if (self = [super init]) {

        _chunkDuration = @(0);

        _cencKey = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPushAVStreamTransportClusterCMAFContainerOptionsStruct alloc] init];

    other.chunkDuration = self.chunkDuration;
    other.cencKey = self.cencKey;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: chunkDuration:%@; cencKey:%@; >", NSStringFromClass([self class]), _chunkDuration, [_cencKey base64EncodedStringWithOptions:0]];
    return descriptionString;
}

@end

@implementation MTRPushAVStreamTransportClusterContainerOptionsStruct
- (instancetype)init
{
    if (self = [super init]) {

        _containerType = @(0);

        _cmafContainerOptions = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPushAVStreamTransportClusterContainerOptionsStruct alloc] init];

    other.containerType = self.containerType;
    other.cmafContainerOptions = self.cmafContainerOptions;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: containerType:%@; cmafContainerOptions:%@; >", NSStringFromClass([self class]), _containerType, _cmafContainerOptions];
    return descriptionString;
}

@end

@implementation MTRPushAVStreamTransportClusterTransportOptionsStruct
- (instancetype)init
{
    if (self = [super init]) {

        _streamUsage = @(0);

        _videoStreamID = nil;

        _audioStreamID = nil;

        _endpointID = @(0);

        _url = @"";

        _triggerOptions = [MTRPushAVStreamTransportClusterTransportTriggerOptionsStruct new];

        _ingestMethod = @(0);

        _containerFormat = @(0);

        _containerOptions = [MTRPushAVStreamTransportClusterContainerOptionsStruct new];

        _metadataOptions = nil;

        _expiryTime = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPushAVStreamTransportClusterTransportOptionsStruct alloc] init];

    other.streamUsage = self.streamUsage;
    other.videoStreamID = self.videoStreamID;
    other.audioStreamID = self.audioStreamID;
    other.endpointID = self.endpointID;
    other.url = self.url;
    other.triggerOptions = self.triggerOptions;
    other.ingestMethod = self.ingestMethod;
    other.containerFormat = self.containerFormat;
    other.containerOptions = self.containerOptions;
    other.metadataOptions = self.metadataOptions;
    other.expiryTime = self.expiryTime;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: streamUsage:%@; videoStreamID:%@; audioStreamID:%@; endpointID:%@; url:%@; triggerOptions:%@; ingestMethod:%@; containerFormat:%@; containerOptions:%@; metadataOptions:%@; expiryTime:%@; >", NSStringFromClass([self class]), _streamUsage, _videoStreamID, _audioStreamID, _endpointID, _url, _triggerOptions, _ingestMethod, _containerFormat, _containerOptions, _metadataOptions, _expiryTime];
    return descriptionString;
}

@end

@implementation MTRPushAVStreamTransportClusterTransportConfigurationStruct
- (instancetype)init
{
    if (self = [super init]) {

        _connectionID = @(0);

        _transportStatus = @(0);

        _transportOptions = [MTRPushAVStreamTransportClusterTransportOptionsStruct new];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPushAVStreamTransportClusterTransportConfigurationStruct alloc] init];

    other.connectionID = self.connectionID;
    other.transportStatus = self.transportStatus;
    other.transportOptions = self.transportOptions;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: connectionID:%@; transportStatus:%@; transportOptions:%@; >", NSStringFromClass([self class]), _connectionID, _transportStatus, _transportOptions];
    return descriptionString;
}

@end

@implementation MTRPushAVStreamTransportClusterPushTransportBeginEvent
- (instancetype)init
{
    if (self = [super init]) {

        _connectionID = @(0);

        _triggerType = @(0);

        _activationReason = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPushAVStreamTransportClusterPushTransportBeginEvent alloc] init];

    other.connectionID = self.connectionID;
    other.triggerType = self.triggerType;
    other.activationReason = self.activationReason;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: connectionID:%@; triggerType:%@; activationReason:%@; >", NSStringFromClass([self class]), _connectionID, _triggerType, _activationReason];
    return descriptionString;
}

@end

@implementation MTRPushAVStreamTransportClusterPushTransportEndEvent
- (instancetype)init
{
    if (self = [super init]) {

        _connectionID = @(0);

        _triggerType = @(0);

        _activationReason = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRPushAVStreamTransportClusterPushTransportEndEvent alloc] init];

    other.connectionID = self.connectionID;
    other.triggerType = self.triggerType;
    other.activationReason = self.activationReason;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: connectionID:%@; triggerType:%@; activationReason:%@; >", NSStringFromClass([self class]), _connectionID, _triggerType, _activationReason];
    return descriptionString;
}

@end

@implementation MTRChimeClusterChimeSoundStruct
- (instancetype)init
{
    if (self = [super init]) {

        _chimeID = @(0);

        _name = @"";
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRChimeClusterChimeSoundStruct alloc] init];

    other.chimeID = self.chimeID;
    other.name = self.name;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: chimeID:%@; name:%@; >", NSStringFromClass([self class]), _chimeID, _name];
    return descriptionString;
}

@end

@implementation MTREcosystemInformationClusterDeviceTypeStruct
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
    auto other = [[MTREcosystemInformationClusterDeviceTypeStruct alloc] init];

    other.deviceType = self.deviceType;
    other.revision = self.revision;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: deviceType:%@; revision:%@; >", NSStringFromClass([self class]), _deviceType, _revision];
    return descriptionString;
}

@end

@implementation MTREcosystemInformationClusterEcosystemDeviceStruct
- (instancetype)init
{
    if (self = [super init]) {

        _deviceName = nil;

        _deviceNameLastEdit = nil;

        _bridgedEndpoint = @(0);

        _originalEndpoint = @(0);

        _deviceTypes = [NSArray array];

        _uniqueLocationIDs = [NSArray array];

        _uniqueLocationIDsLastEdit = @(0);

        _fabricIndex = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTREcosystemInformationClusterEcosystemDeviceStruct alloc] init];

    other.deviceName = self.deviceName;
    other.deviceNameLastEdit = self.deviceNameLastEdit;
    other.bridgedEndpoint = self.bridgedEndpoint;
    other.originalEndpoint = self.originalEndpoint;
    other.deviceTypes = self.deviceTypes;
    other.uniqueLocationIDs = self.uniqueLocationIDs;
    other.uniqueLocationIDsLastEdit = self.uniqueLocationIDsLastEdit;
    other.fabricIndex = self.fabricIndex;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: deviceName:%@; deviceNameLastEdit:%@; bridgedEndpoint:%@; originalEndpoint:%@; deviceTypes:%@; uniqueLocationIDs:%@; uniqueLocationIDsLastEdit:%@; fabricIndex:%@; >", NSStringFromClass([self class]), _deviceName, _deviceNameLastEdit, _bridgedEndpoint, _originalEndpoint, _deviceTypes, _uniqueLocationIDs, _uniqueLocationIDsLastEdit, _fabricIndex];
    return descriptionString;
}

@end

@implementation MTREcosystemInformationClusterEcosystemLocationStruct
- (instancetype)init
{
    if (self = [super init]) {

        _uniqueLocationID = @"";

        _locationDescriptor = [MTRDataTypeLocationDescriptorStruct new];

        _locationDescriptorLastEdit = @(0);

        _fabricIndex = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTREcosystemInformationClusterEcosystemLocationStruct alloc] init];

    other.uniqueLocationID = self.uniqueLocationID;
    other.locationDescriptor = self.locationDescriptor;
    other.locationDescriptorLastEdit = self.locationDescriptorLastEdit;
    other.fabricIndex = self.fabricIndex;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: uniqueLocationID:%@; locationDescriptor:%@; locationDescriptorLastEdit:%@; fabricIndex:%@; >", NSStringFromClass([self class]), _uniqueLocationID, _locationDescriptor, _locationDescriptorLastEdit, _fabricIndex];
    return descriptionString;
}

@end

@implementation MTRCommissionerControlClusterCommissioningRequestResultEvent
- (instancetype)init
{
    if (self = [super init]) {

        _requestID = @(0);

        _clientNodeID = @(0);

        _statusCode = @(0);

        _fabricIndex = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRCommissionerControlClusterCommissioningRequestResultEvent alloc] init];

    other.requestID = self.requestID;
    other.clientNodeID = self.clientNodeID;
    other.statusCode = self.statusCode;
    other.fabricIndex = self.fabricIndex;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: requestID:%@; clientNodeID:%@; statusCode:%@; fabricIndex:%@; >", NSStringFromClass([self class]), _requestID, _clientNodeID, _statusCode, _fabricIndex];
    return descriptionString;
}

@end

@implementation MTRTLSCertificateManagementClusterTLSCertStruct
- (instancetype)init
{
    if (self = [super init]) {

        _caid = @(0);

        _certificate = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRTLSCertificateManagementClusterTLSCertStruct alloc] init];

    other.caid = self.caid;
    other.certificate = self.certificate;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: caid:%@; certificate:%@; >", NSStringFromClass([self class]), _caid, [_certificate base64EncodedStringWithOptions:0]];
    return descriptionString;
}

@end

@implementation MTRTLSCertificateManagementClusterTLSClientCertificateDetailStruct
- (instancetype)init
{
    if (self = [super init]) {

        _ccdid = @(0);

        _clientCertificate = nil;

        _intermediateCertificates = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRTLSCertificateManagementClusterTLSClientCertificateDetailStruct alloc] init];

    other.ccdid = self.ccdid;
    other.clientCertificate = self.clientCertificate;
    other.intermediateCertificates = self.intermediateCertificates;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: ccdid:%@; clientCertificate:%@; intermediateCertificates:%@; >", NSStringFromClass([self class]), _ccdid, [_clientCertificate base64EncodedStringWithOptions:0], _intermediateCertificates];
    return descriptionString;
}

@end

@implementation MTRTLSClientManagementClusterTLSEndpointStruct
- (instancetype)init
{
    if (self = [super init]) {

        _endpointID = @(0);

        _hostname = [NSData data];

        _port = @(0);

        _caid = @(0);

        _ccdid = nil;

        _status = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRTLSClientManagementClusterTLSEndpointStruct alloc] init];

    other.endpointID = self.endpointID;
    other.hostname = self.hostname;
    other.port = self.port;
    other.caid = self.caid;
    other.ccdid = self.ccdid;
    other.status = self.status;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: endpointID:%@; hostname:%@; port:%@; caid:%@; ccdid:%@; status:%@; >", NSStringFromClass([self class]), _endpointID, [_hostname base64EncodedStringWithOptions:0], _port, _caid, _ccdid, _status];
    return descriptionString;
}

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

        _i = nil;
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
    other.i = self.i;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: a:%@; b:%@; c:%@; d:%@; e:%@; f:%@; g:%@; h:%@; i:%@; >", NSStringFromClass([self class]), _a, _b, _c, [_d base64EncodedStringWithOptions:0], _e, _f, _g, _h, _i];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterSimpleStruct : MTRUnitTestingClusterSimpleStruct
@dynamic a;
@dynamic b;
@dynamic c;
@dynamic d;
@dynamic e;
@dynamic f;
@dynamic g;
@dynamic h;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: fabricSensitiveInt8u:%@; optionalFabricSensitiveInt8u:%@; nullableFabricSensitiveInt8u:%@; nullableOptionalFabricSensitiveInt8u:%@; fabricSensitiveCharString:%@; fabricSensitiveStruct:%@; fabricSensitiveInt8uList:%@; fabricIndex:%@; >", NSStringFromClass([self class]), _fabricSensitiveInt8u, _optionalFabricSensitiveInt8u, _nullableFabricSensitiveInt8u, _nullableOptionalFabricSensitiveInt8u, _fabricSensitiveCharString, _fabricSensitiveStruct, _fabricSensitiveInt8uList, _fabricIndex];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterTestFabricScoped : MTRUnitTestingClusterTestFabricScoped
@dynamic fabricSensitiveInt8u;
@dynamic optionalFabricSensitiveInt8u;
@dynamic nullableFabricSensitiveInt8u;
@dynamic nullableOptionalFabricSensitiveInt8u;
@dynamic fabricSensitiveCharString;
@dynamic fabricSensitiveStruct;
@dynamic fabricSensitiveInt8uList;
@dynamic fabricIndex;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: nullableInt:%@; optionalInt:%@; nullableOptionalInt:%@; nullableString:%@; optionalString:%@; nullableOptionalString:%@; nullableStruct:%@; optionalStruct:%@; nullableOptionalStruct:%@; nullableList:%@; optionalList:%@; nullableOptionalList:%@; >", NSStringFromClass([self class]), _nullableInt, _optionalInt, _nullableOptionalInt, _nullableString, _optionalString, _nullableOptionalString, _nullableStruct, _optionalStruct, _nullableOptionalStruct, _nullableList, _optionalList, _nullableOptionalList];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterNullablesAndOptionalsStruct : MTRUnitTestingClusterNullablesAndOptionalsStruct
@dynamic nullableInt;
@dynamic optionalInt;
@dynamic nullableOptionalInt;
@dynamic nullableString;
@dynamic optionalString;
@dynamic nullableOptionalString;
@dynamic nullableStruct;
@dynamic optionalStruct;
@dynamic nullableOptionalStruct;
@dynamic nullableList;
@dynamic optionalList;
@dynamic nullableOptionalList;
@end

@implementation MTRUnitTestingClusterNestedStruct
- (instancetype)init
{
    if (self = [super init]) {

        _a = @(0);

        _b = @(0);

        _c = [MTRUnitTestingClusterSimpleStruct new];

        _d = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRUnitTestingClusterNestedStruct alloc] init];

    other.a = self.a;
    other.b = self.b;
    other.c = self.c;
    other.d = self.d;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: a:%@; b:%@; c:%@; d:%@; >", NSStringFromClass([self class]), _a, _b, _c, _d];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterNestedStruct : MTRUnitTestingClusterNestedStruct
@dynamic a;
@dynamic b;
@dynamic c;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: a:%@; b:%@; c:%@; d:%@; e:%@; f:%@; g:%@; >", NSStringFromClass([self class]), _a, _b, _c, _d, _e, _f, _g];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterNestedStructList : MTRUnitTestingClusterNestedStructList
@dynamic a;
@dynamic b;
@dynamic c;
@dynamic d;
@dynamic e;
@dynamic f;
@dynamic g;
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
@dynamic a;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: member1:%@; member2:%@; >", NSStringFromClass([self class]), _member1, [_member2 base64EncodedStringWithOptions:0]];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterTestListStructOctet : MTRUnitTestingClusterTestListStructOctet
@dynamic member1;
@dynamic member2;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; arg2:%@; arg3:%@; arg4:%@; arg5:%@; arg6:%@; >", NSStringFromClass([self class]), _arg1, _arg2, _arg3, _arg4, _arg5, _arg6];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterTestEventEvent : MTRUnitTestingClusterTestEventEvent
@dynamic arg1;
@dynamic arg2;
@dynamic arg3;
@dynamic arg4;
@dynamic arg5;
@dynamic arg6;
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
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: fabricIndex:%@; >", NSStringFromClass([self class]), _fabricIndex];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterTestFabricScopedEventEvent : MTRUnitTestingClusterTestFabricScopedEventEvent
@dynamic fabricIndex;
@end

@implementation MTRUnitTestingClusterTestDifferentVendorMeiEventEvent
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRUnitTestingClusterTestDifferentVendorMeiEventEvent alloc] init];

    other.arg1 = self.arg1;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; >", NSStringFromClass([self class]), _arg1];
    return descriptionString;
}

@end

@implementation MTRSampleMEIClusterPingCountEventEvent
- (instancetype)init
{
    if (self = [super init]) {

        _count = @(0);

        _fabricIndex = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRSampleMEIClusterPingCountEventEvent alloc] init];

    other.count = self.count;
    other.fabricIndex = self.fabricIndex;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: count:%@; fabricIndex:%@; >", NSStringFromClass([self class]), _count, _fabricIndex];
    return descriptionString;
}

@end

NS_ASSUME_NONNULL_END
