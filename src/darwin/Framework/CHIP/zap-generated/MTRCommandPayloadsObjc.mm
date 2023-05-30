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

#import "MTRCommandPayloadsObjc.h"
#import "MTRBaseDevice_Internal.h"
#import "MTRCommandPayloads_Internal.h"
#import "MTRError_Internal.h"
#import "MTRLogging_Internal.h"
#import "NSDataSpanConversion.h"
#import "NSStringSpanConversion.h"

#include <app/data-model/Decode.h>
#include <lib/core/TLV.h>

NS_ASSUME_NONNULL_BEGIN

@implementation MTRIdentifyClusterIdentifyParams
- (instancetype)init
{
    if (self = [super init]) {

        _identifyTime = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRIdentifyClusterIdentifyParams alloc] init];

    other.identifyTime = self.identifyTime;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: identifyTime:%@; >", NSStringFromClass([self class]), _identifyTime];
    return descriptionString;
}

@end
@implementation MTRIdentifyClusterTriggerEffectParams
- (instancetype)init
{
    if (self = [super init]) {

        _effectIdentifier = @(0);

        _effectVariant = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRIdentifyClusterTriggerEffectParams alloc] init];

    other.effectIdentifier = self.effectIdentifier;
    other.effectVariant = self.effectVariant;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: effectIdentifier:%@; effectVariant:%@; >",
                                             NSStringFromClass([self class]), _effectIdentifier, _effectVariant];
    return descriptionString;
}

@end
@implementation MTRGroupsClusterAddGroupParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupID = @(0);

        _groupName = @"";
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGroupsClusterAddGroupParams alloc] init];

    other.groupID = self.groupID;
    other.groupName = self.groupName;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: groupID:%@; groupName:%@; >", NSStringFromClass([self class]), _groupID, _groupName];
    return descriptionString;
}

@end

@implementation MTRGroupsClusterAddGroupParams (Deprecated)

- (void)setGroupId:(NSNumber * _Nonnull)groupId
{
    self.groupID = groupId;
}

- (NSNumber * _Nonnull)groupId
{
    return self.groupID;
}
@end
@implementation MTRGroupsClusterAddGroupResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _groupID = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGroupsClusterAddGroupResponseParams alloc] init];

    other.status = self.status;
    other.groupID = self.groupID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: status:%@; groupID:%@; >", NSStringFromClass([self class]), _status, _groupID];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::Groups::Commands::AddGroupResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRGroupsClusterAddGroupResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::Groups::Commands::AddGroupResponse::DecodableType &)decodableStruct
{
    {
        self.status = [NSNumber numberWithUnsignedChar:decodableStruct.status];
    }
    {
        self.groupID = [NSNumber numberWithUnsignedShort:decodableStruct.groupID];
    }
    return CHIP_NO_ERROR;
}
@end

@implementation MTRGroupsClusterAddGroupResponseParams (Deprecated)

- (void)setGroupId:(NSNumber * _Nonnull)groupId
{
    self.groupID = groupId;
}

- (NSNumber * _Nonnull)groupId
{
    return self.groupID;
}
@end
@implementation MTRGroupsClusterViewGroupParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupID = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGroupsClusterViewGroupParams alloc] init];

    other.groupID = self.groupID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: groupID:%@; >", NSStringFromClass([self class]), _groupID];
    return descriptionString;
}

@end

@implementation MTRGroupsClusterViewGroupParams (Deprecated)

- (void)setGroupId:(NSNumber * _Nonnull)groupId
{
    self.groupID = groupId;
}

- (NSNumber * _Nonnull)groupId
{
    return self.groupID;
}
@end
@implementation MTRGroupsClusterViewGroupResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _groupID = @(0);

        _groupName = @"";
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGroupsClusterViewGroupResponseParams alloc] init];

    other.status = self.status;
    other.groupID = self.groupID;
    other.groupName = self.groupName;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: status:%@; groupID:%@; groupName:%@; >",
                                             NSStringFromClass([self class]), _status, _groupID, _groupName];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::Groups::Commands::ViewGroupResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRGroupsClusterViewGroupResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::Groups::Commands::ViewGroupResponse::DecodableType &)decodableStruct
{
    {
        self.status = [NSNumber numberWithUnsignedChar:decodableStruct.status];
    }
    {
        self.groupID = [NSNumber numberWithUnsignedShort:decodableStruct.groupID];
    }
    {
        self.groupName = AsString(decodableStruct.groupName);
        if (self.groupName == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            return err;
        }
    }
    return CHIP_NO_ERROR;
}
@end

@implementation MTRGroupsClusterViewGroupResponseParams (Deprecated)

- (void)setGroupId:(NSNumber * _Nonnull)groupId
{
    self.groupID = groupId;
}

- (NSNumber * _Nonnull)groupId
{
    return self.groupID;
}
@end
@implementation MTRGroupsClusterGetGroupMembershipParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupList = [NSArray array];
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGroupsClusterGetGroupMembershipParams alloc] init];

    other.groupList = self.groupList;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: groupList:%@; >", NSStringFromClass([self class]), _groupList];
    return descriptionString;
}

@end
@implementation MTRGroupsClusterGetGroupMembershipResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _capacity = nil;

        _groupList = [NSArray array];
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGroupsClusterGetGroupMembershipResponseParams alloc] init];

    other.capacity = self.capacity;
    other.groupList = self.groupList;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: capacity:%@; groupList:%@; >", NSStringFromClass([self class]), _capacity, _groupList];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::Groups::Commands::GetGroupMembershipResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRGroupsClusterGetGroupMembershipResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::Groups::Commands::GetGroupMembershipResponse::DecodableType &)decodableStruct
{
    {
        if (decodableStruct.capacity.IsNull()) {
            self.capacity = nil;
        } else {
            self.capacity = [NSNumber numberWithUnsignedChar:decodableStruct.capacity.Value()];
        }
    }
    {
        { // Scope for our temporary variables
            auto * array_0 = [NSMutableArray new];
            auto iter_0 = decodableStruct.groupList.begin();
            while (iter_0.Next()) {
                auto & entry_0 = iter_0.GetValue();
                NSNumber * newElement_0;
                newElement_0 = [NSNumber numberWithUnsignedShort:entry_0];
                [array_0 addObject:newElement_0];
            }
            CHIP_ERROR err = iter_0.GetStatus();
            if (err != CHIP_NO_ERROR) {
                return err;
            }
            self.groupList = array_0;
        }
    }
    return CHIP_NO_ERROR;
}
@end
@implementation MTRGroupsClusterRemoveGroupParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupID = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGroupsClusterRemoveGroupParams alloc] init];

    other.groupID = self.groupID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: groupID:%@; >", NSStringFromClass([self class]), _groupID];
    return descriptionString;
}

@end

@implementation MTRGroupsClusterRemoveGroupParams (Deprecated)

- (void)setGroupId:(NSNumber * _Nonnull)groupId
{
    self.groupID = groupId;
}

- (NSNumber * _Nonnull)groupId
{
    return self.groupID;
}
@end
@implementation MTRGroupsClusterRemoveGroupResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _groupID = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGroupsClusterRemoveGroupResponseParams alloc] init];

    other.status = self.status;
    other.groupID = self.groupID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: status:%@; groupID:%@; >", NSStringFromClass([self class]), _status, _groupID];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::Groups::Commands::RemoveGroupResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRGroupsClusterRemoveGroupResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::Groups::Commands::RemoveGroupResponse::DecodableType &)decodableStruct
{
    {
        self.status = [NSNumber numberWithUnsignedChar:decodableStruct.status];
    }
    {
        self.groupID = [NSNumber numberWithUnsignedShort:decodableStruct.groupID];
    }
    return CHIP_NO_ERROR;
}
@end

@implementation MTRGroupsClusterRemoveGroupResponseParams (Deprecated)

- (void)setGroupId:(NSNumber * _Nonnull)groupId
{
    self.groupID = groupId;
}

- (NSNumber * _Nonnull)groupId
{
    return self.groupID;
}
@end
@implementation MTRGroupsClusterRemoveAllGroupsParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGroupsClusterRemoveAllGroupsParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRGroupsClusterAddGroupIfIdentifyingParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupID = @(0);

        _groupName = @"";
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGroupsClusterAddGroupIfIdentifyingParams alloc] init];

    other.groupID = self.groupID;
    other.groupName = self.groupName;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: groupID:%@; groupName:%@; >", NSStringFromClass([self class]), _groupID, _groupName];
    return descriptionString;
}

@end

@implementation MTRGroupsClusterAddGroupIfIdentifyingParams (Deprecated)

- (void)setGroupId:(NSNumber * _Nonnull)groupId
{
    self.groupID = groupId;
}

- (NSNumber * _Nonnull)groupId
{
    return self.groupID;
}
@end
@implementation MTRScenesClusterAddSceneParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupID = @(0);

        _sceneID = @(0);

        _transitionTime = @(0);

        _sceneName = @"";

        _extensionFieldSets = [NSArray array];
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterAddSceneParams alloc] init];

    other.groupID = self.groupID;
    other.sceneID = self.sceneID;
    other.transitionTime = self.transitionTime;
    other.sceneName = self.sceneName;
    other.extensionFieldSets = self.extensionFieldSets;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: groupID:%@; sceneID:%@; transitionTime:%@; sceneName:%@; extensionFieldSets:%@; >",
                  NSStringFromClass([self class]), _groupID, _sceneID, _transitionTime, _sceneName, _extensionFieldSets];
    return descriptionString;
}

@end

@implementation MTRScenesClusterAddSceneParams (Deprecated)

- (void)setGroupId:(NSNumber * _Nonnull)groupId
{
    self.groupID = groupId;
}

- (NSNumber * _Nonnull)groupId
{
    return self.groupID;
}

- (void)setSceneId:(NSNumber * _Nonnull)sceneId
{
    self.sceneID = sceneId;
}

- (NSNumber * _Nonnull)sceneId
{
    return self.sceneID;
}
@end
@implementation MTRScenesClusterAddSceneResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _groupID = @(0);

        _sceneID = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterAddSceneResponseParams alloc] init];

    other.status = self.status;
    other.groupID = self.groupID;
    other.sceneID = self.sceneID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: status:%@; groupID:%@; sceneID:%@; >",
                                             NSStringFromClass([self class]), _status, _groupID, _sceneID];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::Scenes::Commands::AddSceneResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRScenesClusterAddSceneResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::Scenes::Commands::AddSceneResponse::DecodableType &)decodableStruct
{
    {
        self.status = [NSNumber numberWithUnsignedChar:decodableStruct.status];
    }
    {
        self.groupID = [NSNumber numberWithUnsignedShort:decodableStruct.groupID];
    }
    {
        self.sceneID = [NSNumber numberWithUnsignedChar:decodableStruct.sceneID];
    }
    return CHIP_NO_ERROR;
}
@end

@implementation MTRScenesClusterAddSceneResponseParams (Deprecated)

- (void)setGroupId:(NSNumber * _Nonnull)groupId
{
    self.groupID = groupId;
}

- (NSNumber * _Nonnull)groupId
{
    return self.groupID;
}

- (void)setSceneId:(NSNumber * _Nonnull)sceneId
{
    self.sceneID = sceneId;
}

- (NSNumber * _Nonnull)sceneId
{
    return self.sceneID;
}
@end
@implementation MTRScenesClusterViewSceneParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupID = @(0);

        _sceneID = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterViewSceneParams alloc] init];

    other.groupID = self.groupID;
    other.sceneID = self.sceneID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: groupID:%@; sceneID:%@; >", NSStringFromClass([self class]), _groupID, _sceneID];
    return descriptionString;
}

@end

@implementation MTRScenesClusterViewSceneParams (Deprecated)

- (void)setGroupId:(NSNumber * _Nonnull)groupId
{
    self.groupID = groupId;
}

- (NSNumber * _Nonnull)groupId
{
    return self.groupID;
}

- (void)setSceneId:(NSNumber * _Nonnull)sceneId
{
    self.sceneID = sceneId;
}

- (NSNumber * _Nonnull)sceneId
{
    return self.sceneID;
}
@end
@implementation MTRScenesClusterViewSceneResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _groupID = @(0);

        _sceneID = @(0);

        _transitionTime = nil;

        _sceneName = nil;

        _extensionFieldSets = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterViewSceneResponseParams alloc] init];

    other.status = self.status;
    other.groupID = self.groupID;
    other.sceneID = self.sceneID;
    other.transitionTime = self.transitionTime;
    other.sceneName = self.sceneName;
    other.extensionFieldSets = self.extensionFieldSets;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: status:%@; groupID:%@; sceneID:%@; transitionTime:%@; sceneName:%@; extensionFieldSets:%@; >",
        NSStringFromClass([self class]), _status, _groupID, _sceneID, _transitionTime, _sceneName, _extensionFieldSets];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::Scenes::Commands::ViewSceneResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRScenesClusterViewSceneResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::Scenes::Commands::ViewSceneResponse::DecodableType &)decodableStruct
{
    {
        self.status = [NSNumber numberWithUnsignedChar:decodableStruct.status];
    }
    {
        self.groupID = [NSNumber numberWithUnsignedShort:decodableStruct.groupID];
    }
    {
        self.sceneID = [NSNumber numberWithUnsignedChar:decodableStruct.sceneID];
    }
    {
        if (decodableStruct.transitionTime.HasValue()) {
            self.transitionTime = [NSNumber numberWithUnsignedShort:decodableStruct.transitionTime.Value()];
        } else {
            self.transitionTime = nil;
        }
    }
    {
        if (decodableStruct.sceneName.HasValue()) {
            self.sceneName = AsString(decodableStruct.sceneName.Value());
            if (self.sceneName == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                return err;
            }
        } else {
            self.sceneName = nil;
        }
    }
    {
        if (decodableStruct.extensionFieldSets.HasValue()) {
            { // Scope for our temporary variables
                auto * array_1 = [NSMutableArray new];
                auto iter_1 = decodableStruct.extensionFieldSets.Value().begin();
                while (iter_1.Next()) {
                    auto & entry_1 = iter_1.GetValue();
                    MTRScenesClusterExtensionFieldSet * newElement_1;
                    newElement_1 = [MTRScenesClusterExtensionFieldSet new];
                    newElement_1.clusterID = [NSNumber numberWithUnsignedInt:entry_1.clusterID];
                    { // Scope for our temporary variables
                        auto * array_3 = [NSMutableArray new];
                        auto iter_3 = entry_1.attributeValueList.begin();
                        while (iter_3.Next()) {
                            auto & entry_3 = iter_3.GetValue();
                            MTRScenesClusterAttributeValuePair * newElement_3;
                            newElement_3 = [MTRScenesClusterAttributeValuePair new];
                            if (entry_3.attributeID.HasValue()) {
                                newElement_3.attributeID = [NSNumber numberWithUnsignedInt:entry_3.attributeID.Value()];
                            } else {
                                newElement_3.attributeID = nil;
                            }
                            newElement_3.attributeValue = [NSNumber numberWithUnsignedInt:entry_3.attributeValue];
                            [array_3 addObject:newElement_3];
                        }
                        CHIP_ERROR err = iter_3.GetStatus();
                        if (err != CHIP_NO_ERROR) {
                            return err;
                        }
                        newElement_1.attributeValueList = array_3;
                    }
                    [array_1 addObject:newElement_1];
                }
                CHIP_ERROR err = iter_1.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    return err;
                }
                self.extensionFieldSets = array_1;
            }
        } else {
            self.extensionFieldSets = nil;
        }
    }
    return CHIP_NO_ERROR;
}
@end

@implementation MTRScenesClusterViewSceneResponseParams (Deprecated)

- (void)setGroupId:(NSNumber * _Nonnull)groupId
{
    self.groupID = groupId;
}

- (NSNumber * _Nonnull)groupId
{
    return self.groupID;
}

- (void)setSceneId:(NSNumber * _Nonnull)sceneId
{
    self.sceneID = sceneId;
}

- (NSNumber * _Nonnull)sceneId
{
    return self.sceneID;
}
@end
@implementation MTRScenesClusterRemoveSceneParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupID = @(0);

        _sceneID = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterRemoveSceneParams alloc] init];

    other.groupID = self.groupID;
    other.sceneID = self.sceneID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: groupID:%@; sceneID:%@; >", NSStringFromClass([self class]), _groupID, _sceneID];
    return descriptionString;
}

@end

@implementation MTRScenesClusterRemoveSceneParams (Deprecated)

- (void)setGroupId:(NSNumber * _Nonnull)groupId
{
    self.groupID = groupId;
}

- (NSNumber * _Nonnull)groupId
{
    return self.groupID;
}

- (void)setSceneId:(NSNumber * _Nonnull)sceneId
{
    self.sceneID = sceneId;
}

- (NSNumber * _Nonnull)sceneId
{
    return self.sceneID;
}
@end
@implementation MTRScenesClusterRemoveSceneResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _groupID = @(0);

        _sceneID = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterRemoveSceneResponseParams alloc] init];

    other.status = self.status;
    other.groupID = self.groupID;
    other.sceneID = self.sceneID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: status:%@; groupID:%@; sceneID:%@; >",
                                             NSStringFromClass([self class]), _status, _groupID, _sceneID];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::Scenes::Commands::RemoveSceneResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRScenesClusterRemoveSceneResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::Scenes::Commands::RemoveSceneResponse::DecodableType &)decodableStruct
{
    {
        self.status = [NSNumber numberWithUnsignedChar:decodableStruct.status];
    }
    {
        self.groupID = [NSNumber numberWithUnsignedShort:decodableStruct.groupID];
    }
    {
        self.sceneID = [NSNumber numberWithUnsignedChar:decodableStruct.sceneID];
    }
    return CHIP_NO_ERROR;
}
@end

@implementation MTRScenesClusterRemoveSceneResponseParams (Deprecated)

- (void)setGroupId:(NSNumber * _Nonnull)groupId
{
    self.groupID = groupId;
}

- (NSNumber * _Nonnull)groupId
{
    return self.groupID;
}

- (void)setSceneId:(NSNumber * _Nonnull)sceneId
{
    self.sceneID = sceneId;
}

- (NSNumber * _Nonnull)sceneId
{
    return self.sceneID;
}
@end
@implementation MTRScenesClusterRemoveAllScenesParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupID = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterRemoveAllScenesParams alloc] init];

    other.groupID = self.groupID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: groupID:%@; >", NSStringFromClass([self class]), _groupID];
    return descriptionString;
}

@end

@implementation MTRScenesClusterRemoveAllScenesParams (Deprecated)

- (void)setGroupId:(NSNumber * _Nonnull)groupId
{
    self.groupID = groupId;
}

- (NSNumber * _Nonnull)groupId
{
    return self.groupID;
}
@end
@implementation MTRScenesClusterRemoveAllScenesResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _groupID = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterRemoveAllScenesResponseParams alloc] init];

    other.status = self.status;
    other.groupID = self.groupID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: status:%@; groupID:%@; >", NSStringFromClass([self class]), _status, _groupID];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::Scenes::Commands::RemoveAllScenesResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRScenesClusterRemoveAllScenesResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::Scenes::Commands::RemoveAllScenesResponse::DecodableType &)decodableStruct
{
    {
        self.status = [NSNumber numberWithUnsignedChar:decodableStruct.status];
    }
    {
        self.groupID = [NSNumber numberWithUnsignedShort:decodableStruct.groupID];
    }
    return CHIP_NO_ERROR;
}
@end

@implementation MTRScenesClusterRemoveAllScenesResponseParams (Deprecated)

- (void)setGroupId:(NSNumber * _Nonnull)groupId
{
    self.groupID = groupId;
}

- (NSNumber * _Nonnull)groupId
{
    return self.groupID;
}
@end
@implementation MTRScenesClusterStoreSceneParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupID = @(0);

        _sceneID = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterStoreSceneParams alloc] init];

    other.groupID = self.groupID;
    other.sceneID = self.sceneID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: groupID:%@; sceneID:%@; >", NSStringFromClass([self class]), _groupID, _sceneID];
    return descriptionString;
}

@end

@implementation MTRScenesClusterStoreSceneParams (Deprecated)

- (void)setGroupId:(NSNumber * _Nonnull)groupId
{
    self.groupID = groupId;
}

- (NSNumber * _Nonnull)groupId
{
    return self.groupID;
}

- (void)setSceneId:(NSNumber * _Nonnull)sceneId
{
    self.sceneID = sceneId;
}

- (NSNumber * _Nonnull)sceneId
{
    return self.sceneID;
}
@end
@implementation MTRScenesClusterStoreSceneResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _groupID = @(0);

        _sceneID = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterStoreSceneResponseParams alloc] init];

    other.status = self.status;
    other.groupID = self.groupID;
    other.sceneID = self.sceneID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: status:%@; groupID:%@; sceneID:%@; >",
                                             NSStringFromClass([self class]), _status, _groupID, _sceneID];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::Scenes::Commands::StoreSceneResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRScenesClusterStoreSceneResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::Scenes::Commands::StoreSceneResponse::DecodableType &)decodableStruct
{
    {
        self.status = [NSNumber numberWithUnsignedChar:decodableStruct.status];
    }
    {
        self.groupID = [NSNumber numberWithUnsignedShort:decodableStruct.groupID];
    }
    {
        self.sceneID = [NSNumber numberWithUnsignedChar:decodableStruct.sceneID];
    }
    return CHIP_NO_ERROR;
}
@end

@implementation MTRScenesClusterStoreSceneResponseParams (Deprecated)

- (void)setGroupId:(NSNumber * _Nonnull)groupId
{
    self.groupID = groupId;
}

- (NSNumber * _Nonnull)groupId
{
    return self.groupID;
}

- (void)setSceneId:(NSNumber * _Nonnull)sceneId
{
    self.sceneID = sceneId;
}

- (NSNumber * _Nonnull)sceneId
{
    return self.sceneID;
}
@end
@implementation MTRScenesClusterRecallSceneParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupID = @(0);

        _sceneID = @(0);

        _transitionTime = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterRecallSceneParams alloc] init];

    other.groupID = self.groupID;
    other.sceneID = self.sceneID;
    other.transitionTime = self.transitionTime;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: groupID:%@; sceneID:%@; transitionTime:%@; >",
                                             NSStringFromClass([self class]), _groupID, _sceneID, _transitionTime];
    return descriptionString;
}

@end

@implementation MTRScenesClusterRecallSceneParams (Deprecated)

- (void)setGroupId:(NSNumber * _Nonnull)groupId
{
    self.groupID = groupId;
}

- (NSNumber * _Nonnull)groupId
{
    return self.groupID;
}

- (void)setSceneId:(NSNumber * _Nonnull)sceneId
{
    self.sceneID = sceneId;
}

- (NSNumber * _Nonnull)sceneId
{
    return self.sceneID;
}
@end
@implementation MTRScenesClusterGetSceneMembershipParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupID = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterGetSceneMembershipParams alloc] init];

    other.groupID = self.groupID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: groupID:%@; >", NSStringFromClass([self class]), _groupID];
    return descriptionString;
}

@end

@implementation MTRScenesClusterGetSceneMembershipParams (Deprecated)

- (void)setGroupId:(NSNumber * _Nonnull)groupId
{
    self.groupID = groupId;
}

- (NSNumber * _Nonnull)groupId
{
    return self.groupID;
}
@end
@implementation MTRScenesClusterGetSceneMembershipResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _capacity = nil;

        _groupID = @(0);

        _sceneList = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterGetSceneMembershipResponseParams alloc] init];

    other.status = self.status;
    other.capacity = self.capacity;
    other.groupID = self.groupID;
    other.sceneList = self.sceneList;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: status:%@; capacity:%@; groupID:%@; sceneList:%@; >",
                                             NSStringFromClass([self class]), _status, _capacity, _groupID, _sceneList];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::Scenes::Commands::GetSceneMembershipResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRScenesClusterGetSceneMembershipResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::Scenes::Commands::GetSceneMembershipResponse::DecodableType &)decodableStruct
{
    {
        self.status = [NSNumber numberWithUnsignedChar:decodableStruct.status];
    }
    {
        if (decodableStruct.capacity.IsNull()) {
            self.capacity = nil;
        } else {
            self.capacity = [NSNumber numberWithUnsignedChar:decodableStruct.capacity.Value()];
        }
    }
    {
        self.groupID = [NSNumber numberWithUnsignedShort:decodableStruct.groupID];
    }
    {
        if (decodableStruct.sceneList.HasValue()) {
            { // Scope for our temporary variables
                auto * array_1 = [NSMutableArray new];
                auto iter_1 = decodableStruct.sceneList.Value().begin();
                while (iter_1.Next()) {
                    auto & entry_1 = iter_1.GetValue();
                    NSNumber * newElement_1;
                    newElement_1 = [NSNumber numberWithUnsignedChar:entry_1];
                    [array_1 addObject:newElement_1];
                }
                CHIP_ERROR err = iter_1.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    return err;
                }
                self.sceneList = array_1;
            }
        } else {
            self.sceneList = nil;
        }
    }
    return CHIP_NO_ERROR;
}
@end

@implementation MTRScenesClusterGetSceneMembershipResponseParams (Deprecated)

- (void)setGroupId:(NSNumber * _Nonnull)groupId
{
    self.groupID = groupId;
}

- (NSNumber * _Nonnull)groupId
{
    return self.groupID;
}
@end
@implementation MTRScenesClusterEnhancedAddSceneParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupID = @(0);

        _sceneID = @(0);

        _transitionTime = @(0);

        _sceneName = @"";

        _extensionFieldSets = [NSArray array];
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterEnhancedAddSceneParams alloc] init];

    other.groupID = self.groupID;
    other.sceneID = self.sceneID;
    other.transitionTime = self.transitionTime;
    other.sceneName = self.sceneName;
    other.extensionFieldSets = self.extensionFieldSets;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: groupID:%@; sceneID:%@; transitionTime:%@; sceneName:%@; extensionFieldSets:%@; >",
                  NSStringFromClass([self class]), _groupID, _sceneID, _transitionTime, _sceneName, _extensionFieldSets];
    return descriptionString;
}

@end

@implementation MTRScenesClusterEnhancedAddSceneParams (Deprecated)

- (void)setGroupId:(NSNumber * _Nonnull)groupId
{
    self.groupID = groupId;
}

- (NSNumber * _Nonnull)groupId
{
    return self.groupID;
}

- (void)setSceneId:(NSNumber * _Nonnull)sceneId
{
    self.sceneID = sceneId;
}

- (NSNumber * _Nonnull)sceneId
{
    return self.sceneID;
}
@end
@implementation MTRScenesClusterEnhancedAddSceneResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _groupID = @(0);

        _sceneID = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterEnhancedAddSceneResponseParams alloc] init];

    other.status = self.status;
    other.groupID = self.groupID;
    other.sceneID = self.sceneID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: status:%@; groupID:%@; sceneID:%@; >",
                                             NSStringFromClass([self class]), _status, _groupID, _sceneID];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::Scenes::Commands::EnhancedAddSceneResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRScenesClusterEnhancedAddSceneResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::Scenes::Commands::EnhancedAddSceneResponse::DecodableType &)decodableStruct
{
    {
        self.status = [NSNumber numberWithUnsignedChar:decodableStruct.status];
    }
    {
        self.groupID = [NSNumber numberWithUnsignedShort:decodableStruct.groupID];
    }
    {
        self.sceneID = [NSNumber numberWithUnsignedChar:decodableStruct.sceneID];
    }
    return CHIP_NO_ERROR;
}
@end

@implementation MTRScenesClusterEnhancedAddSceneResponseParams (Deprecated)

- (void)setGroupId:(NSNumber * _Nonnull)groupId
{
    self.groupID = groupId;
}

- (NSNumber * _Nonnull)groupId
{
    return self.groupID;
}

- (void)setSceneId:(NSNumber * _Nonnull)sceneId
{
    self.sceneID = sceneId;
}

- (NSNumber * _Nonnull)sceneId
{
    return self.sceneID;
}
@end
@implementation MTRScenesClusterEnhancedViewSceneParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupID = @(0);

        _sceneID = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterEnhancedViewSceneParams alloc] init];

    other.groupID = self.groupID;
    other.sceneID = self.sceneID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: groupID:%@; sceneID:%@; >", NSStringFromClass([self class]), _groupID, _sceneID];
    return descriptionString;
}

@end

@implementation MTRScenesClusterEnhancedViewSceneParams (Deprecated)

- (void)setGroupId:(NSNumber * _Nonnull)groupId
{
    self.groupID = groupId;
}

- (NSNumber * _Nonnull)groupId
{
    return self.groupID;
}

- (void)setSceneId:(NSNumber * _Nonnull)sceneId
{
    self.sceneID = sceneId;
}

- (NSNumber * _Nonnull)sceneId
{
    return self.sceneID;
}
@end
@implementation MTRScenesClusterEnhancedViewSceneResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _groupID = @(0);

        _sceneID = @(0);

        _transitionTime = nil;

        _sceneName = nil;

        _extensionFieldSets = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterEnhancedViewSceneResponseParams alloc] init];

    other.status = self.status;
    other.groupID = self.groupID;
    other.sceneID = self.sceneID;
    other.transitionTime = self.transitionTime;
    other.sceneName = self.sceneName;
    other.extensionFieldSets = self.extensionFieldSets;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: status:%@; groupID:%@; sceneID:%@; transitionTime:%@; sceneName:%@; extensionFieldSets:%@; >",
        NSStringFromClass([self class]), _status, _groupID, _sceneID, _transitionTime, _sceneName, _extensionFieldSets];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::Scenes::Commands::EnhancedViewSceneResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRScenesClusterEnhancedViewSceneResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::Scenes::Commands::EnhancedViewSceneResponse::DecodableType &)decodableStruct
{
    {
        self.status = [NSNumber numberWithUnsignedChar:decodableStruct.status];
    }
    {
        self.groupID = [NSNumber numberWithUnsignedShort:decodableStruct.groupID];
    }
    {
        self.sceneID = [NSNumber numberWithUnsignedChar:decodableStruct.sceneID];
    }
    {
        if (decodableStruct.transitionTime.HasValue()) {
            self.transitionTime = [NSNumber numberWithUnsignedShort:decodableStruct.transitionTime.Value()];
        } else {
            self.transitionTime = nil;
        }
    }
    {
        if (decodableStruct.sceneName.HasValue()) {
            self.sceneName = AsString(decodableStruct.sceneName.Value());
            if (self.sceneName == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                return err;
            }
        } else {
            self.sceneName = nil;
        }
    }
    {
        if (decodableStruct.extensionFieldSets.HasValue()) {
            { // Scope for our temporary variables
                auto * array_1 = [NSMutableArray new];
                auto iter_1 = decodableStruct.extensionFieldSets.Value().begin();
                while (iter_1.Next()) {
                    auto & entry_1 = iter_1.GetValue();
                    MTRScenesClusterExtensionFieldSet * newElement_1;
                    newElement_1 = [MTRScenesClusterExtensionFieldSet new];
                    newElement_1.clusterID = [NSNumber numberWithUnsignedInt:entry_1.clusterID];
                    { // Scope for our temporary variables
                        auto * array_3 = [NSMutableArray new];
                        auto iter_3 = entry_1.attributeValueList.begin();
                        while (iter_3.Next()) {
                            auto & entry_3 = iter_3.GetValue();
                            MTRScenesClusterAttributeValuePair * newElement_3;
                            newElement_3 = [MTRScenesClusterAttributeValuePair new];
                            if (entry_3.attributeID.HasValue()) {
                                newElement_3.attributeID = [NSNumber numberWithUnsignedInt:entry_3.attributeID.Value()];
                            } else {
                                newElement_3.attributeID = nil;
                            }
                            newElement_3.attributeValue = [NSNumber numberWithUnsignedInt:entry_3.attributeValue];
                            [array_3 addObject:newElement_3];
                        }
                        CHIP_ERROR err = iter_3.GetStatus();
                        if (err != CHIP_NO_ERROR) {
                            return err;
                        }
                        newElement_1.attributeValueList = array_3;
                    }
                    [array_1 addObject:newElement_1];
                }
                CHIP_ERROR err = iter_1.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    return err;
                }
                self.extensionFieldSets = array_1;
            }
        } else {
            self.extensionFieldSets = nil;
        }
    }
    return CHIP_NO_ERROR;
}
@end

@implementation MTRScenesClusterEnhancedViewSceneResponseParams (Deprecated)

- (void)setGroupId:(NSNumber * _Nonnull)groupId
{
    self.groupID = groupId;
}

- (NSNumber * _Nonnull)groupId
{
    return self.groupID;
}

- (void)setSceneId:(NSNumber * _Nonnull)sceneId
{
    self.sceneID = sceneId;
}

- (NSNumber * _Nonnull)sceneId
{
    return self.sceneID;
}
@end
@implementation MTRScenesClusterCopySceneParams
- (instancetype)init
{
    if (self = [super init]) {

        _mode = @(0);

        _groupIdentifierFrom = @(0);

        _sceneIdentifierFrom = @(0);

        _groupIdentifierTo = @(0);

        _sceneIdentifierTo = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterCopySceneParams alloc] init];

    other.mode = self.mode;
    other.groupIdentifierFrom = self.groupIdentifierFrom;
    other.sceneIdentifierFrom = self.sceneIdentifierFrom;
    other.groupIdentifierTo = self.groupIdentifierTo;
    other.sceneIdentifierTo = self.sceneIdentifierTo;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:
            @"<%@: mode:%@; groupIdentifierFrom:%@; sceneIdentifierFrom:%@; groupIdentifierTo:%@; sceneIdentifierTo:%@; >",
        NSStringFromClass([self class]), _mode, _groupIdentifierFrom, _sceneIdentifierFrom, _groupIdentifierTo, _sceneIdentifierTo];
    return descriptionString;
}

@end

@implementation MTRScenesClusterCopySceneParams (Deprecated)

- (void)setGroupIdFrom:(NSNumber * _Nonnull)groupIdFrom
{
    self.groupIdentifierFrom = groupIdFrom;
}

- (NSNumber * _Nonnull)groupIdFrom
{
    return self.groupIdentifierFrom;
}

- (void)setSceneIdFrom:(NSNumber * _Nonnull)sceneIdFrom
{
    self.sceneIdentifierFrom = sceneIdFrom;
}

- (NSNumber * _Nonnull)sceneIdFrom
{
    return self.sceneIdentifierFrom;
}

- (void)setGroupIdTo:(NSNumber * _Nonnull)groupIdTo
{
    self.groupIdentifierTo = groupIdTo;
}

- (NSNumber * _Nonnull)groupIdTo
{
    return self.groupIdentifierTo;
}

- (void)setSceneIdTo:(NSNumber * _Nonnull)sceneIdTo
{
    self.sceneIdentifierTo = sceneIdTo;
}

- (NSNumber * _Nonnull)sceneIdTo
{
    return self.sceneIdentifierTo;
}
@end
@implementation MTRScenesClusterCopySceneResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _groupIdentifierFrom = @(0);

        _sceneIdentifierFrom = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterCopySceneResponseParams alloc] init];

    other.status = self.status;
    other.groupIdentifierFrom = self.groupIdentifierFrom;
    other.sceneIdentifierFrom = self.sceneIdentifierFrom;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: status:%@; groupIdentifierFrom:%@; sceneIdentifierFrom:%@; >",
                                             NSStringFromClass([self class]), _status, _groupIdentifierFrom, _sceneIdentifierFrom];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::Scenes::Commands::CopySceneResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRScenesClusterCopySceneResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::Scenes::Commands::CopySceneResponse::DecodableType &)decodableStruct
{
    {
        self.status = [NSNumber numberWithUnsignedChar:decodableStruct.status];
    }
    {
        self.groupIdentifierFrom = [NSNumber numberWithUnsignedShort:decodableStruct.groupIdentifierFrom];
    }
    {
        self.sceneIdentifierFrom = [NSNumber numberWithUnsignedChar:decodableStruct.sceneIdentifierFrom];
    }
    return CHIP_NO_ERROR;
}
@end

@implementation MTRScenesClusterCopySceneResponseParams (Deprecated)

- (void)setGroupIdFrom:(NSNumber * _Nonnull)groupIdFrom
{
    self.groupIdentifierFrom = groupIdFrom;
}

- (NSNumber * _Nonnull)groupIdFrom
{
    return self.groupIdentifierFrom;
}

- (void)setSceneIdFrom:(NSNumber * _Nonnull)sceneIdFrom
{
    self.sceneIdentifierFrom = sceneIdFrom;
}

- (NSNumber * _Nonnull)sceneIdFrom
{
    return self.sceneIdentifierFrom;
}
@end
@implementation MTROnOffClusterOffParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROnOffClusterOffParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTROnOffClusterOnParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROnOffClusterOnParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTROnOffClusterToggleParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROnOffClusterToggleParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTROnOffClusterOffWithEffectParams
- (instancetype)init
{
    if (self = [super init]) {

        _effectIdentifier = @(0);

        _effectVariant = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROnOffClusterOffWithEffectParams alloc] init];

    other.effectIdentifier = self.effectIdentifier;
    other.effectVariant = self.effectVariant;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: effectIdentifier:%@; effectVariant:%@; >",
                                             NSStringFromClass([self class]), _effectIdentifier, _effectVariant];
    return descriptionString;
}

@end

@implementation MTROnOffClusterOffWithEffectParams (Deprecated)

- (void)setEffectId:(NSNumber * _Nonnull)effectId
{
    self.effectIdentifier = effectId;
}

- (NSNumber * _Nonnull)effectId
{
    return self.effectIdentifier;
}
@end
@implementation MTROnOffClusterOnWithRecallGlobalSceneParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROnOffClusterOnWithRecallGlobalSceneParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTROnOffClusterOnWithTimedOffParams
- (instancetype)init
{
    if (self = [super init]) {

        _onOffControl = @(0);

        _onTime = @(0);

        _offWaitTime = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROnOffClusterOnWithTimedOffParams alloc] init];

    other.onOffControl = self.onOffControl;
    other.onTime = self.onTime;
    other.offWaitTime = self.offWaitTime;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: onOffControl:%@; onTime:%@; offWaitTime:%@; >",
                                             NSStringFromClass([self class]), _onOffControl, _onTime, _offWaitTime];
    return descriptionString;
}

@end
@implementation MTRLevelControlClusterMoveToLevelParams
- (instancetype)init
{
    if (self = [super init]) {

        _level = @(0);

        _transitionTime = nil;

        _optionsMask = @(0);

        _optionsOverride = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRLevelControlClusterMoveToLevelParams alloc] init];

    other.level = self.level;
    other.transitionTime = self.transitionTime;
    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: level:%@; transitionTime:%@; optionsMask:%@; optionsOverride:%@; >",
                  NSStringFromClass([self class]), _level, _transitionTime, _optionsMask, _optionsOverride];
    return descriptionString;
}

@end
@implementation MTRLevelControlClusterMoveParams
- (instancetype)init
{
    if (self = [super init]) {

        _moveMode = @(0);

        _rate = nil;

        _optionsMask = @(0);

        _optionsOverride = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRLevelControlClusterMoveParams alloc] init];

    other.moveMode = self.moveMode;
    other.rate = self.rate;
    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: moveMode:%@; rate:%@; optionsMask:%@; optionsOverride:%@; >",
                                             NSStringFromClass([self class]), _moveMode, _rate, _optionsMask, _optionsOverride];
    return descriptionString;
}

@end
@implementation MTRLevelControlClusterStepParams
- (instancetype)init
{
    if (self = [super init]) {

        _stepMode = @(0);

        _stepSize = @(0);

        _transitionTime = nil;

        _optionsMask = @(0);

        _optionsOverride = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRLevelControlClusterStepParams alloc] init];

    other.stepMode = self.stepMode;
    other.stepSize = self.stepSize;
    other.transitionTime = self.transitionTime;
    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: stepMode:%@; stepSize:%@; transitionTime:%@; optionsMask:%@; optionsOverride:%@; >",
                  NSStringFromClass([self class]), _stepMode, _stepSize, _transitionTime, _optionsMask, _optionsOverride];
    return descriptionString;
}

@end
@implementation MTRLevelControlClusterStopParams
- (instancetype)init
{
    if (self = [super init]) {

        _optionsMask = @(0);

        _optionsOverride = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRLevelControlClusterStopParams alloc] init];

    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: optionsMask:%@; optionsOverride:%@; >",
                                             NSStringFromClass([self class]), _optionsMask, _optionsOverride];
    return descriptionString;
}

@end
@implementation MTRLevelControlClusterMoveToLevelWithOnOffParams
- (instancetype)init
{
    if (self = [super init]) {

        _level = @(0);

        _transitionTime = nil;

        _optionsMask = @(0);

        _optionsOverride = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRLevelControlClusterMoveToLevelWithOnOffParams alloc] init];

    other.level = self.level;
    other.transitionTime = self.transitionTime;
    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: level:%@; transitionTime:%@; optionsMask:%@; optionsOverride:%@; >",
                  NSStringFromClass([self class]), _level, _transitionTime, _optionsMask, _optionsOverride];
    return descriptionString;
}

@end
@implementation MTRLevelControlClusterMoveWithOnOffParams
- (instancetype)init
{
    if (self = [super init]) {

        _moveMode = @(0);

        _rate = nil;

        _optionsMask = @(0);

        _optionsOverride = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRLevelControlClusterMoveWithOnOffParams alloc] init];

    other.moveMode = self.moveMode;
    other.rate = self.rate;
    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: moveMode:%@; rate:%@; optionsMask:%@; optionsOverride:%@; >",
                                             NSStringFromClass([self class]), _moveMode, _rate, _optionsMask, _optionsOverride];
    return descriptionString;
}

@end
@implementation MTRLevelControlClusterStepWithOnOffParams
- (instancetype)init
{
    if (self = [super init]) {

        _stepMode = @(0);

        _stepSize = @(0);

        _transitionTime = nil;

        _optionsMask = @(0);

        _optionsOverride = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRLevelControlClusterStepWithOnOffParams alloc] init];

    other.stepMode = self.stepMode;
    other.stepSize = self.stepSize;
    other.transitionTime = self.transitionTime;
    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: stepMode:%@; stepSize:%@; transitionTime:%@; optionsMask:%@; optionsOverride:%@; >",
                  NSStringFromClass([self class]), _stepMode, _stepSize, _transitionTime, _optionsMask, _optionsOverride];
    return descriptionString;
}

@end
@implementation MTRLevelControlClusterStopWithOnOffParams
- (instancetype)init
{
    if (self = [super init]) {

        _optionsMask = @(0);

        _optionsOverride = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRLevelControlClusterStopWithOnOffParams alloc] init];

    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: optionsMask:%@; optionsOverride:%@; >",
                                             NSStringFromClass([self class]), _optionsMask, _optionsOverride];
    return descriptionString;
}

@end
@implementation MTRLevelControlClusterMoveToClosestFrequencyParams
- (instancetype)init
{
    if (self = [super init]) {

        _frequency = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRLevelControlClusterMoveToClosestFrequencyParams alloc] init];

    other.frequency = self.frequency;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: frequency:%@; >", NSStringFromClass([self class]), _frequency];
    return descriptionString;
}

@end
@implementation MTRActionsClusterInstantActionParams
- (instancetype)init
{
    if (self = [super init]) {

        _actionID = @(0);

        _invokeID = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRActionsClusterInstantActionParams alloc] init];

    other.actionID = self.actionID;
    other.invokeID = self.invokeID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: actionID:%@; invokeID:%@; >", NSStringFromClass([self class]), _actionID, _invokeID];
    return descriptionString;
}

@end
@implementation MTRActionsClusterInstantActionWithTransitionParams
- (instancetype)init
{
    if (self = [super init]) {

        _actionID = @(0);

        _invokeID = nil;

        _transitionTime = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRActionsClusterInstantActionWithTransitionParams alloc] init];

    other.actionID = self.actionID;
    other.invokeID = self.invokeID;
    other.transitionTime = self.transitionTime;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: actionID:%@; invokeID:%@; transitionTime:%@; >",
                                             NSStringFromClass([self class]), _actionID, _invokeID, _transitionTime];
    return descriptionString;
}

@end
@implementation MTRActionsClusterStartActionParams
- (instancetype)init
{
    if (self = [super init]) {

        _actionID = @(0);

        _invokeID = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRActionsClusterStartActionParams alloc] init];

    other.actionID = self.actionID;
    other.invokeID = self.invokeID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: actionID:%@; invokeID:%@; >", NSStringFromClass([self class]), _actionID, _invokeID];
    return descriptionString;
}

@end
@implementation MTRActionsClusterStartActionWithDurationParams
- (instancetype)init
{
    if (self = [super init]) {

        _actionID = @(0);

        _invokeID = nil;

        _duration = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRActionsClusterStartActionWithDurationParams alloc] init];

    other.actionID = self.actionID;
    other.invokeID = self.invokeID;
    other.duration = self.duration;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: actionID:%@; invokeID:%@; duration:%@; >",
                                             NSStringFromClass([self class]), _actionID, _invokeID, _duration];
    return descriptionString;
}

@end
@implementation MTRActionsClusterStopActionParams
- (instancetype)init
{
    if (self = [super init]) {

        _actionID = @(0);

        _invokeID = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRActionsClusterStopActionParams alloc] init];

    other.actionID = self.actionID;
    other.invokeID = self.invokeID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: actionID:%@; invokeID:%@; >", NSStringFromClass([self class]), _actionID, _invokeID];
    return descriptionString;
}

@end
@implementation MTRActionsClusterPauseActionParams
- (instancetype)init
{
    if (self = [super init]) {

        _actionID = @(0);

        _invokeID = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRActionsClusterPauseActionParams alloc] init];

    other.actionID = self.actionID;
    other.invokeID = self.invokeID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: actionID:%@; invokeID:%@; >", NSStringFromClass([self class]), _actionID, _invokeID];
    return descriptionString;
}

@end
@implementation MTRActionsClusterPauseActionWithDurationParams
- (instancetype)init
{
    if (self = [super init]) {

        _actionID = @(0);

        _invokeID = nil;

        _duration = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRActionsClusterPauseActionWithDurationParams alloc] init];

    other.actionID = self.actionID;
    other.invokeID = self.invokeID;
    other.duration = self.duration;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: actionID:%@; invokeID:%@; duration:%@; >",
                                             NSStringFromClass([self class]), _actionID, _invokeID, _duration];
    return descriptionString;
}

@end
@implementation MTRActionsClusterResumeActionParams
- (instancetype)init
{
    if (self = [super init]) {

        _actionID = @(0);

        _invokeID = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRActionsClusterResumeActionParams alloc] init];

    other.actionID = self.actionID;
    other.invokeID = self.invokeID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: actionID:%@; invokeID:%@; >", NSStringFromClass([self class]), _actionID, _invokeID];
    return descriptionString;
}

@end
@implementation MTRActionsClusterEnableActionParams
- (instancetype)init
{
    if (self = [super init]) {

        _actionID = @(0);

        _invokeID = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRActionsClusterEnableActionParams alloc] init];

    other.actionID = self.actionID;
    other.invokeID = self.invokeID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: actionID:%@; invokeID:%@; >", NSStringFromClass([self class]), _actionID, _invokeID];
    return descriptionString;
}

@end
@implementation MTRActionsClusterEnableActionWithDurationParams
- (instancetype)init
{
    if (self = [super init]) {

        _actionID = @(0);

        _invokeID = nil;

        _duration = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRActionsClusterEnableActionWithDurationParams alloc] init];

    other.actionID = self.actionID;
    other.invokeID = self.invokeID;
    other.duration = self.duration;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: actionID:%@; invokeID:%@; duration:%@; >",
                                             NSStringFromClass([self class]), _actionID, _invokeID, _duration];
    return descriptionString;
}

@end
@implementation MTRActionsClusterDisableActionParams
- (instancetype)init
{
    if (self = [super init]) {

        _actionID = @(0);

        _invokeID = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRActionsClusterDisableActionParams alloc] init];

    other.actionID = self.actionID;
    other.invokeID = self.invokeID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: actionID:%@; invokeID:%@; >", NSStringFromClass([self class]), _actionID, _invokeID];
    return descriptionString;
}

@end
@implementation MTRActionsClusterDisableActionWithDurationParams
- (instancetype)init
{
    if (self = [super init]) {

        _actionID = @(0);

        _invokeID = nil;

        _duration = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRActionsClusterDisableActionWithDurationParams alloc] init];

    other.actionID = self.actionID;
    other.invokeID = self.invokeID;
    other.duration = self.duration;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: actionID:%@; invokeID:%@; duration:%@; >",
                                             NSStringFromClass([self class]), _actionID, _invokeID, _duration];
    return descriptionString;
}

@end
@implementation MTRBasicClusterMfgSpecificPingParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRBasicClusterMfgSpecificPingParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTROTASoftwareUpdateProviderClusterQueryImageParams
- (instancetype)init
{
    if (self = [super init]) {

        _vendorID = @(0);

        _productID = @(0);

        _softwareVersion = @(0);

        _protocolsSupported = [NSArray array];

        _hardwareVersion = nil;

        _location = nil;

        _requestorCanConsent = nil;

        _metadataForProvider = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROTASoftwareUpdateProviderClusterQueryImageParams alloc] init];

    other.vendorID = self.vendorID;
    other.productID = self.productID;
    other.softwareVersion = self.softwareVersion;
    other.protocolsSupported = self.protocolsSupported;
    other.hardwareVersion = self.hardwareVersion;
    other.location = self.location;
    other.requestorCanConsent = self.requestorCanConsent;
    other.metadataForProvider = self.metadataForProvider;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: vendorID:%@; productID:%@; softwareVersion:%@; protocolsSupported:%@; "
                                   @"hardwareVersion:%@; location:%@; requestorCanConsent:%@; metadataForProvider:%@; >",
                  NSStringFromClass([self class]), _vendorID, _productID, _softwareVersion, _protocolsSupported, _hardwareVersion,
                  _location, _requestorCanConsent, [_metadataForProvider base64EncodedStringWithOptions:0]];
    return descriptionString;
}

@end

@implementation MTROtaSoftwareUpdateProviderClusterQueryImageParams
@end

@implementation MTROTASoftwareUpdateProviderClusterQueryImageParams (Deprecated)

- (void)setVendorId:(NSNumber * _Nonnull)vendorId
{
    self.vendorID = vendorId;
}

- (NSNumber * _Nonnull)vendorId
{
    return self.vendorID;
}

- (void)setProductId:(NSNumber * _Nonnull)productId
{
    self.productID = productId;
}

- (NSNumber * _Nonnull)productId
{
    return self.productID;
}
@end
@implementation MTROTASoftwareUpdateProviderClusterQueryImageResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _delayedActionTime = nil;

        _imageURI = nil;

        _softwareVersion = nil;

        _softwareVersionString = nil;

        _updateToken = nil;

        _userConsentNeeded = nil;

        _metadataForRequestor = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROTASoftwareUpdateProviderClusterQueryImageResponseParams alloc] init];

    other.status = self.status;
    other.delayedActionTime = self.delayedActionTime;
    other.imageURI = self.imageURI;
    other.softwareVersion = self.softwareVersion;
    other.softwareVersionString = self.softwareVersionString;
    other.updateToken = self.updateToken;
    other.userConsentNeeded = self.userConsentNeeded;
    other.metadataForRequestor = self.metadataForRequestor;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: status:%@; delayedActionTime:%@; imageURI:%@; softwareVersion:%@; "
                                   @"softwareVersionString:%@; updateToken:%@; userConsentNeeded:%@; metadataForRequestor:%@; >",
                  NSStringFromClass([self class]), _status, _delayedActionTime, _imageURI, _softwareVersion, _softwareVersionString,
                  [_updateToken base64EncodedStringWithOptions:0], _userConsentNeeded,
                  [_metadataForRequestor base64EncodedStringWithOptions:0]];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImageResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTROTASoftwareUpdateProviderClusterQueryImageResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImageResponse::DecodableType &)decodableStruct
{
    {
        self.status = [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.status)];
    }
    {
        if (decodableStruct.delayedActionTime.HasValue()) {
            self.delayedActionTime = [NSNumber numberWithUnsignedInt:decodableStruct.delayedActionTime.Value()];
        } else {
            self.delayedActionTime = nil;
        }
    }
    {
        if (decodableStruct.imageURI.HasValue()) {
            self.imageURI = AsString(decodableStruct.imageURI.Value());
            if (self.imageURI == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                return err;
            }
        } else {
            self.imageURI = nil;
        }
    }
    {
        if (decodableStruct.softwareVersion.HasValue()) {
            self.softwareVersion = [NSNumber numberWithUnsignedInt:decodableStruct.softwareVersion.Value()];
        } else {
            self.softwareVersion = nil;
        }
    }
    {
        if (decodableStruct.softwareVersionString.HasValue()) {
            self.softwareVersionString = AsString(decodableStruct.softwareVersionString.Value());
            if (self.softwareVersionString == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                return err;
            }
        } else {
            self.softwareVersionString = nil;
        }
    }
    {
        if (decodableStruct.updateToken.HasValue()) {
            self.updateToken = AsData(decodableStruct.updateToken.Value());
        } else {
            self.updateToken = nil;
        }
    }
    {
        if (decodableStruct.userConsentNeeded.HasValue()) {
            self.userConsentNeeded = [NSNumber numberWithBool:decodableStruct.userConsentNeeded.Value()];
        } else {
            self.userConsentNeeded = nil;
        }
    }
    {
        if (decodableStruct.metadataForRequestor.HasValue()) {
            self.metadataForRequestor = AsData(decodableStruct.metadataForRequestor.Value());
        } else {
            self.metadataForRequestor = nil;
        }
    }
    return CHIP_NO_ERROR;
}
@end

@implementation MTROtaSoftwareUpdateProviderClusterQueryImageResponseParams
@end
@implementation MTROTASoftwareUpdateProviderClusterApplyUpdateRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _updateToken = [NSData data];

        _newVersion = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROTASoftwareUpdateProviderClusterApplyUpdateRequestParams alloc] init];

    other.updateToken = self.updateToken;
    other.newVersion = self.newVersion;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: updateToken:%@; newVersion:%@; >", NSStringFromClass([self class]),
                  [_updateToken base64EncodedStringWithOptions:0], _newVersion];
    return descriptionString;
}

@end

@implementation MTROtaSoftwareUpdateProviderClusterApplyUpdateRequestParams
@end
@implementation MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _action = @(0);

        _delayedActionTime = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams alloc] init];

    other.action = self.action;
    other.delayedActionTime = self.delayedActionTime;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: action:%@; delayedActionTime:%@; >", NSStringFromClass([self class]), _action, _delayedActionTime];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateResponse::DecodableType &)decodableStruct
{
    {
        self.action = [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.action)];
    }
    {
        self.delayedActionTime = [NSNumber numberWithUnsignedInt:decodableStruct.delayedActionTime];
    }
    return CHIP_NO_ERROR;
}
@end

@implementation MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams
@end
@implementation MTROTASoftwareUpdateProviderClusterNotifyUpdateAppliedParams
- (instancetype)init
{
    if (self = [super init]) {

        _updateToken = [NSData data];

        _softwareVersion = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROTASoftwareUpdateProviderClusterNotifyUpdateAppliedParams alloc] init];

    other.updateToken = self.updateToken;
    other.softwareVersion = self.softwareVersion;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: updateToken:%@; softwareVersion:%@; >", NSStringFromClass([self class]),
                  [_updateToken base64EncodedStringWithOptions:0], _softwareVersion];
    return descriptionString;
}

@end

@implementation MTROtaSoftwareUpdateProviderClusterNotifyUpdateAppliedParams
@end
@implementation MTROTASoftwareUpdateRequestorClusterAnnounceOTAProviderParams
- (instancetype)init
{
    if (self = [super init]) {

        _providerNodeID = @(0);

        _vendorID = @(0);

        _announcementReason = @(0);

        _metadataForNode = nil;

        _endpoint = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROTASoftwareUpdateRequestorClusterAnnounceOTAProviderParams alloc] init];

    other.providerNodeID = self.providerNodeID;
    other.vendorID = self.vendorID;
    other.announcementReason = self.announcementReason;
    other.metadataForNode = self.metadataForNode;
    other.endpoint = self.endpoint;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: providerNodeID:%@; vendorID:%@; announcementReason:%@; metadataForNode:%@; endpoint:%@; >",
        NSStringFromClass([self class]), _providerNodeID, _vendorID, _announcementReason,
        [_metadataForNode base64EncodedStringWithOptions:0], _endpoint];
    return descriptionString;
}

@end

@implementation MTROtaSoftwareUpdateRequestorClusterAnnounceOtaProviderParams
@end

@implementation MTROTASoftwareUpdateRequestorClusterAnnounceOTAProviderParams (Deprecated)

- (void)setProviderNodeId:(NSNumber * _Nonnull)providerNodeId
{
    self.providerNodeID = providerNodeId;
}

- (NSNumber * _Nonnull)providerNodeId
{
    return self.providerNodeID;
}

- (void)setVendorId:(NSNumber * _Nonnull)vendorId
{
    self.vendorID = vendorId;
}

- (NSNumber * _Nonnull)vendorId
{
    return self.vendorID;
}
@end
@implementation MTRGeneralCommissioningClusterArmFailSafeParams
- (instancetype)init
{
    if (self = [super init]) {

        _expiryLengthSeconds = @(0);

        _breadcrumb = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGeneralCommissioningClusterArmFailSafeParams alloc] init];

    other.expiryLengthSeconds = self.expiryLengthSeconds;
    other.breadcrumb = self.breadcrumb;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: expiryLengthSeconds:%@; breadcrumb:%@; >",
                                             NSStringFromClass([self class]), _expiryLengthSeconds, _breadcrumb];
    return descriptionString;
}

@end
@implementation MTRGeneralCommissioningClusterArmFailSafeResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _errorCode = @(0);

        _debugText = @"";
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGeneralCommissioningClusterArmFailSafeResponseParams alloc] init];

    other.errorCode = self.errorCode;
    other.debugText = self.debugText;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: errorCode:%@; debugText:%@; >", NSStringFromClass([self class]), _errorCode, _debugText];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::GeneralCommissioning::Commands::ArmFailSafeResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRGeneralCommissioningClusterArmFailSafeResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::GeneralCommissioning::Commands::ArmFailSafeResponse::DecodableType &)decodableStruct
{
    {
        self.errorCode = [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.errorCode)];
    }
    {
        self.debugText = AsString(decodableStruct.debugText);
        if (self.debugText == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            return err;
        }
    }
    return CHIP_NO_ERROR;
}
@end
@implementation MTRGeneralCommissioningClusterSetRegulatoryConfigParams
- (instancetype)init
{
    if (self = [super init]) {

        _newRegulatoryConfig = @(0);

        _countryCode = @"";

        _breadcrumb = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGeneralCommissioningClusterSetRegulatoryConfigParams alloc] init];

    other.newRegulatoryConfig = self.newRegulatoryConfig;
    other.countryCode = self.countryCode;
    other.breadcrumb = self.breadcrumb;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: newRegulatoryConfig:%@; countryCode:%@; breadcrumb:%@; >",
                                             NSStringFromClass([self class]), _newRegulatoryConfig, _countryCode, _breadcrumb];
    return descriptionString;
}

@end
@implementation MTRGeneralCommissioningClusterSetRegulatoryConfigResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _errorCode = @(0);

        _debugText = @"";
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGeneralCommissioningClusterSetRegulatoryConfigResponseParams alloc] init];

    other.errorCode = self.errorCode;
    other.debugText = self.debugText;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: errorCode:%@; debugText:%@; >", NSStringFromClass([self class]), _errorCode, _debugText];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::GeneralCommissioning::Commands::SetRegulatoryConfigResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRGeneralCommissioningClusterSetRegulatoryConfigResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::GeneralCommissioning::Commands::SetRegulatoryConfigResponse::DecodableType &)decodableStruct
{
    {
        self.errorCode = [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.errorCode)];
    }
    {
        self.debugText = AsString(decodableStruct.debugText);
        if (self.debugText == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            return err;
        }
    }
    return CHIP_NO_ERROR;
}
@end
@implementation MTRGeneralCommissioningClusterCommissioningCompleteParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGeneralCommissioningClusterCommissioningCompleteParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRGeneralCommissioningClusterCommissioningCompleteResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _errorCode = @(0);

        _debugText = @"";
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGeneralCommissioningClusterCommissioningCompleteResponseParams alloc] init];

    other.errorCode = self.errorCode;
    other.debugText = self.debugText;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: errorCode:%@; debugText:%@; >", NSStringFromClass([self class]), _errorCode, _debugText];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::GeneralCommissioning::Commands::CommissioningCompleteResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRGeneralCommissioningClusterCommissioningCompleteResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::GeneralCommissioning::Commands::CommissioningCompleteResponse::DecodableType &)decodableStruct
{
    {
        self.errorCode = [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.errorCode)];
    }
    {
        self.debugText = AsString(decodableStruct.debugText);
        if (self.debugText == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            return err;
        }
    }
    return CHIP_NO_ERROR;
}
@end
@implementation MTRNetworkCommissioningClusterScanNetworksParams
- (instancetype)init
{
    if (self = [super init]) {

        _ssid = nil;

        _breadcrumb = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRNetworkCommissioningClusterScanNetworksParams alloc] init];

    other.ssid = self.ssid;
    other.breadcrumb = self.breadcrumb;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: ssid:%@; breadcrumb:%@; >", NSStringFromClass([self class]),
                                             [_ssid base64EncodedStringWithOptions:0], _breadcrumb];
    return descriptionString;
}

@end
@implementation MTRNetworkCommissioningClusterScanNetworksResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _networkingStatus = @(0);

        _debugText = nil;

        _wiFiScanResults = nil;

        _threadScanResults = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRNetworkCommissioningClusterScanNetworksResponseParams alloc] init];

    other.networkingStatus = self.networkingStatus;
    other.debugText = self.debugText;
    other.wiFiScanResults = self.wiFiScanResults;
    other.threadScanResults = self.threadScanResults;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: networkingStatus:%@; debugText:%@; wiFiScanResults:%@; threadScanResults:%@; >",
                  NSStringFromClass([self class]), _networkingStatus, _debugText, _wiFiScanResults, _threadScanResults];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::NetworkCommissioning::Commands::ScanNetworksResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRNetworkCommissioningClusterScanNetworksResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::NetworkCommissioning::Commands::ScanNetworksResponse::DecodableType &)decodableStruct
{
    {
        self.networkingStatus = [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.networkingStatus)];
    }
    {
        if (decodableStruct.debugText.HasValue()) {
            self.debugText = AsString(decodableStruct.debugText.Value());
            if (self.debugText == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                return err;
            }
        } else {
            self.debugText = nil;
        }
    }
    {
        if (decodableStruct.wiFiScanResults.HasValue()) {
            { // Scope for our temporary variables
                auto * array_1 = [NSMutableArray new];
                auto iter_1 = decodableStruct.wiFiScanResults.Value().begin();
                while (iter_1.Next()) {
                    auto & entry_1 = iter_1.GetValue();
                    MTRNetworkCommissioningClusterWiFiInterfaceScanResult * newElement_1;
                    newElement_1 = [MTRNetworkCommissioningClusterWiFiInterfaceScanResult new];
                    newElement_1.security = [NSNumber numberWithUnsignedChar:entry_1.security.Raw()];
                    newElement_1.ssid = AsData(entry_1.ssid);
                    newElement_1.bssid = AsData(entry_1.bssid);
                    newElement_1.channel = [NSNumber numberWithUnsignedShort:entry_1.channel];
                    newElement_1.wiFiBand = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_1.wiFiBand)];
                    newElement_1.rssi = [NSNumber numberWithChar:entry_1.rssi];
                    [array_1 addObject:newElement_1];
                }
                CHIP_ERROR err = iter_1.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    return err;
                }
                self.wiFiScanResults = array_1;
            }
        } else {
            self.wiFiScanResults = nil;
        }
    }
    {
        if (decodableStruct.threadScanResults.HasValue()) {
            { // Scope for our temporary variables
                auto * array_1 = [NSMutableArray new];
                auto iter_1 = decodableStruct.threadScanResults.Value().begin();
                while (iter_1.Next()) {
                    auto & entry_1 = iter_1.GetValue();
                    MTRNetworkCommissioningClusterThreadInterfaceScanResult * newElement_1;
                    newElement_1 = [MTRNetworkCommissioningClusterThreadInterfaceScanResult new];
                    newElement_1.panId = [NSNumber numberWithUnsignedShort:entry_1.panId];
                    newElement_1.extendedPanId = [NSNumber numberWithUnsignedLongLong:entry_1.extendedPanId];
                    newElement_1.networkName = AsString(entry_1.networkName);
                    if (newElement_1.networkName == nil) {
                        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                        return err;
                    }
                    newElement_1.channel = [NSNumber numberWithUnsignedShort:entry_1.channel];
                    newElement_1.version = [NSNumber numberWithUnsignedChar:entry_1.version];
                    newElement_1.extendedAddress = AsData(entry_1.extendedAddress);
                    newElement_1.rssi = [NSNumber numberWithChar:entry_1.rssi];
                    newElement_1.lqi = [NSNumber numberWithUnsignedChar:entry_1.lqi];
                    [array_1 addObject:newElement_1];
                }
                CHIP_ERROR err = iter_1.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    return err;
                }
                self.threadScanResults = array_1;
            }
        } else {
            self.threadScanResults = nil;
        }
    }
    return CHIP_NO_ERROR;
}
@end
@implementation MTRNetworkCommissioningClusterAddOrUpdateWiFiNetworkParams
- (instancetype)init
{
    if (self = [super init]) {

        _ssid = [NSData data];

        _credentials = [NSData data];

        _breadcrumb = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRNetworkCommissioningClusterAddOrUpdateWiFiNetworkParams alloc] init];

    other.ssid = self.ssid;
    other.credentials = self.credentials;
    other.breadcrumb = self.breadcrumb;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: ssid:%@; credentials:%@; breadcrumb:%@; >", NSStringFromClass([self class]),
                  [_ssid base64EncodedStringWithOptions:0], [_credentials base64EncodedStringWithOptions:0], _breadcrumb];
    return descriptionString;
}

@end
@implementation MTRNetworkCommissioningClusterAddOrUpdateThreadNetworkParams
- (instancetype)init
{
    if (self = [super init]) {

        _operationalDataset = [NSData data];

        _breadcrumb = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRNetworkCommissioningClusterAddOrUpdateThreadNetworkParams alloc] init];

    other.operationalDataset = self.operationalDataset;
    other.breadcrumb = self.breadcrumb;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: operationalDataset:%@; breadcrumb:%@; >", NSStringFromClass([self class]),
                  [_operationalDataset base64EncodedStringWithOptions:0], _breadcrumb];
    return descriptionString;
}

@end
@implementation MTRNetworkCommissioningClusterRemoveNetworkParams
- (instancetype)init
{
    if (self = [super init]) {

        _networkID = [NSData data];

        _breadcrumb = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRNetworkCommissioningClusterRemoveNetworkParams alloc] init];

    other.networkID = self.networkID;
    other.breadcrumb = self.breadcrumb;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: networkID:%@; breadcrumb:%@; >", NSStringFromClass([self class]),
                  [_networkID base64EncodedStringWithOptions:0], _breadcrumb];
    return descriptionString;
}

@end
@implementation MTRNetworkCommissioningClusterNetworkConfigResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _networkingStatus = @(0);

        _debugText = nil;

        _networkIndex = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRNetworkCommissioningClusterNetworkConfigResponseParams alloc] init];

    other.networkingStatus = self.networkingStatus;
    other.debugText = self.debugText;
    other.networkIndex = self.networkIndex;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: networkingStatus:%@; debugText:%@; networkIndex:%@; >",
                                             NSStringFromClass([self class]), _networkingStatus, _debugText, _networkIndex];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::NetworkCommissioning::Commands::NetworkConfigResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRNetworkCommissioningClusterNetworkConfigResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::NetworkCommissioning::Commands::NetworkConfigResponse::DecodableType &)decodableStruct
{
    {
        self.networkingStatus = [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.networkingStatus)];
    }
    {
        if (decodableStruct.debugText.HasValue()) {
            self.debugText = AsString(decodableStruct.debugText.Value());
            if (self.debugText == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                return err;
            }
        } else {
            self.debugText = nil;
        }
    }
    {
        if (decodableStruct.networkIndex.HasValue()) {
            self.networkIndex = [NSNumber numberWithUnsignedChar:decodableStruct.networkIndex.Value()];
        } else {
            self.networkIndex = nil;
        }
    }
    return CHIP_NO_ERROR;
}
@end
@implementation MTRNetworkCommissioningClusterConnectNetworkParams
- (instancetype)init
{
    if (self = [super init]) {

        _networkID = [NSData data];

        _breadcrumb = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRNetworkCommissioningClusterConnectNetworkParams alloc] init];

    other.networkID = self.networkID;
    other.breadcrumb = self.breadcrumb;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: networkID:%@; breadcrumb:%@; >", NSStringFromClass([self class]),
                  [_networkID base64EncodedStringWithOptions:0], _breadcrumb];
    return descriptionString;
}

@end
@implementation MTRNetworkCommissioningClusterConnectNetworkResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _networkingStatus = @(0);

        _debugText = nil;

        _errorValue = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRNetworkCommissioningClusterConnectNetworkResponseParams alloc] init];

    other.networkingStatus = self.networkingStatus;
    other.debugText = self.debugText;
    other.errorValue = self.errorValue;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: networkingStatus:%@; debugText:%@; errorValue:%@; >",
                                             NSStringFromClass([self class]), _networkingStatus, _debugText, _errorValue];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::NetworkCommissioning::Commands::ConnectNetworkResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRNetworkCommissioningClusterConnectNetworkResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::NetworkCommissioning::Commands::ConnectNetworkResponse::DecodableType &)decodableStruct
{
    {
        self.networkingStatus = [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.networkingStatus)];
    }
    {
        if (decodableStruct.debugText.HasValue()) {
            self.debugText = AsString(decodableStruct.debugText.Value());
            if (self.debugText == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                return err;
            }
        } else {
            self.debugText = nil;
        }
    }
    {
        if (decodableStruct.errorValue.IsNull()) {
            self.errorValue = nil;
        } else {
            self.errorValue = [NSNumber numberWithInt:decodableStruct.errorValue.Value()];
        }
    }
    return CHIP_NO_ERROR;
}
@end
@implementation MTRNetworkCommissioningClusterReorderNetworkParams
- (instancetype)init
{
    if (self = [super init]) {

        _networkID = [NSData data];

        _networkIndex = @(0);

        _breadcrumb = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRNetworkCommissioningClusterReorderNetworkParams alloc] init];

    other.networkID = self.networkID;
    other.networkIndex = self.networkIndex;
    other.breadcrumb = self.breadcrumb;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: networkID:%@; networkIndex:%@; breadcrumb:%@; >", NSStringFromClass([self class]),
                  [_networkID base64EncodedStringWithOptions:0], _networkIndex, _breadcrumb];
    return descriptionString;
}

@end
@implementation MTRDiagnosticLogsClusterRetrieveLogsRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _intent = @(0);

        _requestedProtocol = @(0);

        _transferFileDesignator = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDiagnosticLogsClusterRetrieveLogsRequestParams alloc] init];

    other.intent = self.intent;
    other.requestedProtocol = self.requestedProtocol;
    other.transferFileDesignator = self.transferFileDesignator;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: intent:%@; requestedProtocol:%@; transferFileDesignator:%@; >",
                                             NSStringFromClass([self class]), _intent, _requestedProtocol, _transferFileDesignator];
    return descriptionString;
}

@end
@implementation MTRDiagnosticLogsClusterRetrieveLogsResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _logContent = [NSData data];

        _utcTimeStamp = nil;

        _timeSinceBoot = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDiagnosticLogsClusterRetrieveLogsResponseParams alloc] init];

    other.status = self.status;
    other.logContent = self.logContent;
    other.utcTimeStamp = self.utcTimeStamp;
    other.timeSinceBoot = self.timeSinceBoot;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: status:%@; logContent:%@; utcTimeStamp:%@; timeSinceBoot:%@; >", NSStringFromClass([self class]),
        _status, [_logContent base64EncodedStringWithOptions:0], _utcTimeStamp, _timeSinceBoot];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::DiagnosticLogs::Commands::RetrieveLogsResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRDiagnosticLogsClusterRetrieveLogsResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::DiagnosticLogs::Commands::RetrieveLogsResponse::DecodableType &)decodableStruct
{
    {
        self.status = [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.status)];
    }
    {
        self.logContent = AsData(decodableStruct.logContent);
    }
    {
        if (decodableStruct.UTCTimeStamp.HasValue()) {
            self.utcTimeStamp = [NSNumber numberWithUnsignedLongLong:decodableStruct.UTCTimeStamp.Value()];
        } else {
            self.utcTimeStamp = nil;
        }
    }
    {
        if (decodableStruct.timeSinceBoot.HasValue()) {
            self.timeSinceBoot = [NSNumber numberWithUnsignedLongLong:decodableStruct.timeSinceBoot.Value()];
        } else {
            self.timeSinceBoot = nil;
        }
    }
    return CHIP_NO_ERROR;
}
@end

@implementation MTRDiagnosticLogsClusterRetrieveLogsResponseParams (Deprecated)

- (void)setContent:(NSData * _Nonnull)content
{
    self.logContent = content;
}

- (NSData * _Nonnull)content
{
    return self.logContent;
}

- (void)setTimeStamp:(NSNumber * _Nullable)timeStamp
{
    self.utcTimeStamp = timeStamp;
}

- (NSNumber * _Nullable)timeStamp
{
    return self.utcTimeStamp;
}
@end
@implementation MTRGeneralDiagnosticsClusterTestEventTriggerParams
- (instancetype)init
{
    if (self = [super init]) {

        _enableKey = [NSData data];

        _eventTrigger = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGeneralDiagnosticsClusterTestEventTriggerParams alloc] init];

    other.enableKey = self.enableKey;
    other.eventTrigger = self.eventTrigger;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: enableKey:%@; eventTrigger:%@; >", NSStringFromClass([self class]),
                  [_enableKey base64EncodedStringWithOptions:0], _eventTrigger];
    return descriptionString;
}

@end
@implementation MTRSoftwareDiagnosticsClusterResetWatermarksParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRSoftwareDiagnosticsClusterResetWatermarksParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRThreadNetworkDiagnosticsClusterResetCountsParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRThreadNetworkDiagnosticsClusterResetCountsParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRWiFiNetworkDiagnosticsClusterResetCountsParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRWiFiNetworkDiagnosticsClusterResetCountsParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTREthernetNetworkDiagnosticsClusterResetCountsParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTREthernetNetworkDiagnosticsClusterResetCountsParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRTimeSynchronizationClusterSetUtcTimeParams
- (instancetype)init
{
    if (self = [super init]) {

        _utcTime = @(0);

        _granularity = @(0);

        _timeSource = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRTimeSynchronizationClusterSetUtcTimeParams alloc] init];

    other.utcTime = self.utcTime;
    other.granularity = self.granularity;
    other.timeSource = self.timeSource;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: utcTime:%@; granularity:%@; timeSource:%@; >",
                                             NSStringFromClass([self class]), _utcTime, _granularity, _timeSource];
    return descriptionString;
}

@end
@implementation MTRAdministratorCommissioningClusterOpenCommissioningWindowParams
- (instancetype)init
{
    if (self = [super init]) {

        _commissioningTimeout = @(0);

        _pakePasscodeVerifier = [NSData data];

        _discriminator = @(0);

        _iterations = @(0);

        _salt = [NSData data];
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRAdministratorCommissioningClusterOpenCommissioningWindowParams alloc] init];

    other.commissioningTimeout = self.commissioningTimeout;
    other.pakePasscodeVerifier = self.pakePasscodeVerifier;
    other.discriminator = self.discriminator;
    other.iterations = self.iterations;
    other.salt = self.salt;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: commissioningTimeout:%@; pakePasscodeVerifier:%@; discriminator:%@; iterations:%@; salt:%@; >",
        NSStringFromClass([self class]), _commissioningTimeout, [_pakePasscodeVerifier base64EncodedStringWithOptions:0],
        _discriminator, _iterations, [_salt base64EncodedStringWithOptions:0]];
    return descriptionString;
}

@end

@implementation MTRAdministratorCommissioningClusterOpenCommissioningWindowParams (Deprecated)

- (void)setPakeVerifier:(NSData * _Nonnull)pakeVerifier
{
    self.pakePasscodeVerifier = pakeVerifier;
}

- (NSData * _Nonnull)pakeVerifier
{
    return self.pakePasscodeVerifier;
}
@end
@implementation MTRAdministratorCommissioningClusterOpenBasicCommissioningWindowParams
- (instancetype)init
{
    if (self = [super init]) {

        _commissioningTimeout = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRAdministratorCommissioningClusterOpenBasicCommissioningWindowParams alloc] init];

    other.commissioningTimeout = self.commissioningTimeout;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: commissioningTimeout:%@; >", NSStringFromClass([self class]), _commissioningTimeout];
    return descriptionString;
}

@end
@implementation MTRAdministratorCommissioningClusterRevokeCommissioningParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRAdministratorCommissioningClusterRevokeCommissioningParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTROperationalCredentialsClusterAttestationRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _attestationNonce = [NSData data];
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROperationalCredentialsClusterAttestationRequestParams alloc] init];

    other.attestationNonce = self.attestationNonce;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: attestationNonce:%@; >", NSStringFromClass([self class]),
                                             [_attestationNonce base64EncodedStringWithOptions:0]];
    return descriptionString;
}

@end
@implementation MTROperationalCredentialsClusterAttestationResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _attestationElements = [NSData data];

        _attestationSignature = [NSData data];
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROperationalCredentialsClusterAttestationResponseParams alloc] init];

    other.attestationElements = self.attestationElements;
    other.attestationSignature = self.attestationSignature;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: attestationElements:%@; attestationSignature:%@; >", NSStringFromClass([self class]),
        [_attestationElements base64EncodedStringWithOptions:0], [_attestationSignature base64EncodedStringWithOptions:0]];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::OperationalCredentials::Commands::AttestationResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTROperationalCredentialsClusterAttestationResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::OperationalCredentials::Commands::AttestationResponse::DecodableType &)decodableStruct
{
    {
        self.attestationElements = AsData(decodableStruct.attestationElements);
    }
    {
        self.attestationSignature = AsData(decodableStruct.attestationSignature);
    }
    return CHIP_NO_ERROR;
}
@end

@implementation MTROperationalCredentialsClusterAttestationResponseParams (Deprecated)

- (void)setSignature:(NSData * _Nonnull)signature
{
    self.attestationSignature = signature;
}

- (NSData * _Nonnull)signature
{
    return self.attestationSignature;
}
@end
@implementation MTROperationalCredentialsClusterCertificateChainRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _certificateType = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROperationalCredentialsClusterCertificateChainRequestParams alloc] init];

    other.certificateType = self.certificateType;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: certificateType:%@; >", NSStringFromClass([self class]), _certificateType];
    return descriptionString;
}

@end
@implementation MTROperationalCredentialsClusterCertificateChainResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _certificate = [NSData data];
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROperationalCredentialsClusterCertificateChainResponseParams alloc] init];

    other.certificate = self.certificate;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: certificate:%@; >", NSStringFromClass([self class]),
                                             [_certificate base64EncodedStringWithOptions:0]];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::OperationalCredentials::Commands::CertificateChainResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTROperationalCredentialsClusterCertificateChainResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::OperationalCredentials::Commands::CertificateChainResponse::DecodableType &)decodableStruct
{
    {
        self.certificate = AsData(decodableStruct.certificate);
    }
    return CHIP_NO_ERROR;
}
@end
@implementation MTROperationalCredentialsClusterCSRRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _csrNonce = [NSData data];

        _isForUpdateNOC = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROperationalCredentialsClusterCSRRequestParams alloc] init];

    other.csrNonce = self.csrNonce;
    other.isForUpdateNOC = self.isForUpdateNOC;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: csrNonce:%@; isForUpdateNOC:%@; >", NSStringFromClass([self class]),
                  [_csrNonce base64EncodedStringWithOptions:0], _isForUpdateNOC];
    return descriptionString;
}

@end
@implementation MTROperationalCredentialsClusterCSRResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _nocsrElements = [NSData data];

        _attestationSignature = [NSData data];
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROperationalCredentialsClusterCSRResponseParams alloc] init];

    other.nocsrElements = self.nocsrElements;
    other.attestationSignature = self.attestationSignature;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: nocsrElements:%@; attestationSignature:%@; >", NSStringFromClass([self class]),
                  [_nocsrElements base64EncodedStringWithOptions:0], [_attestationSignature base64EncodedStringWithOptions:0]];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::OperationalCredentials::Commands::CSRResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTROperationalCredentialsClusterCSRResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::OperationalCredentials::Commands::CSRResponse::DecodableType &)decodableStruct
{
    {
        self.nocsrElements = AsData(decodableStruct.NOCSRElements);
    }
    {
        self.attestationSignature = AsData(decodableStruct.attestationSignature);
    }
    return CHIP_NO_ERROR;
}
@end
@implementation MTROperationalCredentialsClusterAddNOCParams
- (instancetype)init
{
    if (self = [super init]) {

        _nocValue = [NSData data];

        _icacValue = nil;

        _ipkValue = [NSData data];

        _caseAdminSubject = @(0);

        _adminVendorId = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROperationalCredentialsClusterAddNOCParams alloc] init];

    other.nocValue = self.nocValue;
    other.icacValue = self.icacValue;
    other.ipkValue = self.ipkValue;
    other.caseAdminSubject = self.caseAdminSubject;
    other.adminVendorId = self.adminVendorId;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: nocValue:%@; icacValue:%@; ipkValue:%@; caseAdminSubject:%@; adminVendorId:%@; >",
                  NSStringFromClass([self class]), [_nocValue base64EncodedStringWithOptions:0],
                  [_icacValue base64EncodedStringWithOptions:0], [_ipkValue base64EncodedStringWithOptions:0], _caseAdminSubject,
                  _adminVendorId];
    return descriptionString;
}

@end
@implementation MTROperationalCredentialsClusterUpdateNOCParams
- (instancetype)init
{
    if (self = [super init]) {

        _nocValue = [NSData data];

        _icacValue = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROperationalCredentialsClusterUpdateNOCParams alloc] init];

    other.nocValue = self.nocValue;
    other.icacValue = self.icacValue;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: nocValue:%@; icacValue:%@; >", NSStringFromClass([self class]),
                  [_nocValue base64EncodedStringWithOptions:0], [_icacValue base64EncodedStringWithOptions:0]];
    return descriptionString;
}

@end
@implementation MTROperationalCredentialsClusterNOCResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _statusCode = @(0);

        _fabricIndex = nil;

        _debugText = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROperationalCredentialsClusterNOCResponseParams alloc] init];

    other.statusCode = self.statusCode;
    other.fabricIndex = self.fabricIndex;
    other.debugText = self.debugText;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: statusCode:%@; fabricIndex:%@; debugText:%@; >",
                                             NSStringFromClass([self class]), _statusCode, _fabricIndex, _debugText];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::OperationalCredentials::Commands::NOCResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTROperationalCredentialsClusterNOCResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::OperationalCredentials::Commands::NOCResponse::DecodableType &)decodableStruct
{
    {
        self.statusCode = [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.statusCode)];
    }
    {
        if (decodableStruct.fabricIndex.HasValue()) {
            self.fabricIndex = [NSNumber numberWithUnsignedChar:decodableStruct.fabricIndex.Value()];
        } else {
            self.fabricIndex = nil;
        }
    }
    {
        if (decodableStruct.debugText.HasValue()) {
            self.debugText = AsString(decodableStruct.debugText.Value());
            if (self.debugText == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                return err;
            }
        } else {
            self.debugText = nil;
        }
    }
    return CHIP_NO_ERROR;
}
@end
@implementation MTROperationalCredentialsClusterUpdateFabricLabelParams
- (instancetype)init
{
    if (self = [super init]) {

        _label = @"";
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROperationalCredentialsClusterUpdateFabricLabelParams alloc] init];

    other.label = self.label;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: label:%@; >", NSStringFromClass([self class]), _label];
    return descriptionString;
}

@end
@implementation MTROperationalCredentialsClusterRemoveFabricParams
- (instancetype)init
{
    if (self = [super init]) {

        _fabricIndex = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROperationalCredentialsClusterRemoveFabricParams alloc] init];

    other.fabricIndex = self.fabricIndex;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: fabricIndex:%@; >", NSStringFromClass([self class]), _fabricIndex];
    return descriptionString;
}

@end
@implementation MTROperationalCredentialsClusterAddTrustedRootCertificateParams
- (instancetype)init
{
    if (self = [super init]) {

        _rootCACertificate = [NSData data];
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROperationalCredentialsClusterAddTrustedRootCertificateParams alloc] init];

    other.rootCACertificate = self.rootCACertificate;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: rootCACertificate:%@; >", NSStringFromClass([self class]),
                                             [_rootCACertificate base64EncodedStringWithOptions:0]];
    return descriptionString;
}

@end

@implementation MTROperationalCredentialsClusterAddTrustedRootCertificateParams (Deprecated)

- (void)setRootCertificate:(NSData * _Nonnull)rootCertificate
{
    self.rootCACertificate = rootCertificate;
}

- (NSData * _Nonnull)rootCertificate
{
    return self.rootCACertificate;
}
@end
@implementation MTRGroupKeyManagementClusterKeySetWriteParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupKeySet = [MTRGroupKeyManagementClusterGroupKeySetStruct new];
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGroupKeyManagementClusterKeySetWriteParams alloc] init];

    other.groupKeySet = self.groupKeySet;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: groupKeySet:%@; >", NSStringFromClass([self class]), _groupKeySet];
    return descriptionString;
}

@end
@implementation MTRGroupKeyManagementClusterKeySetReadParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupKeySetID = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGroupKeyManagementClusterKeySetReadParams alloc] init];

    other.groupKeySetID = self.groupKeySetID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: groupKeySetID:%@; >", NSStringFromClass([self class]), _groupKeySetID];
    return descriptionString;
}

@end
@implementation MTRGroupKeyManagementClusterKeySetReadResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupKeySet = [MTRGroupKeyManagementClusterGroupKeySetStruct new];
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGroupKeyManagementClusterKeySetReadResponseParams alloc] init];

    other.groupKeySet = self.groupKeySet;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: groupKeySet:%@; >", NSStringFromClass([self class]), _groupKeySet];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::GroupKeyManagement::Commands::KeySetReadResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRGroupKeyManagementClusterKeySetReadResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::GroupKeyManagement::Commands::KeySetReadResponse::DecodableType &)decodableStruct
{
    {
        self.groupKeySet = [MTRGroupKeyManagementClusterGroupKeySetStruct new];
        self.groupKeySet.groupKeySetID = [NSNumber numberWithUnsignedShort:decodableStruct.groupKeySet.groupKeySetID];
        self.groupKeySet.groupKeySecurityPolicy =
            [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.groupKeySet.groupKeySecurityPolicy)];
        if (decodableStruct.groupKeySet.epochKey0.IsNull()) {
            self.groupKeySet.epochKey0 = nil;
        } else {
            self.groupKeySet.epochKey0 = AsData(decodableStruct.groupKeySet.epochKey0.Value());
        }
        if (decodableStruct.groupKeySet.epochStartTime0.IsNull()) {
            self.groupKeySet.epochStartTime0 = nil;
        } else {
            self.groupKeySet.epochStartTime0 =
                [NSNumber numberWithUnsignedLongLong:decodableStruct.groupKeySet.epochStartTime0.Value()];
        }
        if (decodableStruct.groupKeySet.epochKey1.IsNull()) {
            self.groupKeySet.epochKey1 = nil;
        } else {
            self.groupKeySet.epochKey1 = AsData(decodableStruct.groupKeySet.epochKey1.Value());
        }
        if (decodableStruct.groupKeySet.epochStartTime1.IsNull()) {
            self.groupKeySet.epochStartTime1 = nil;
        } else {
            self.groupKeySet.epochStartTime1 =
                [NSNumber numberWithUnsignedLongLong:decodableStruct.groupKeySet.epochStartTime1.Value()];
        }
        if (decodableStruct.groupKeySet.epochKey2.IsNull()) {
            self.groupKeySet.epochKey2 = nil;
        } else {
            self.groupKeySet.epochKey2 = AsData(decodableStruct.groupKeySet.epochKey2.Value());
        }
        if (decodableStruct.groupKeySet.epochStartTime2.IsNull()) {
            self.groupKeySet.epochStartTime2 = nil;
        } else {
            self.groupKeySet.epochStartTime2 =
                [NSNumber numberWithUnsignedLongLong:decodableStruct.groupKeySet.epochStartTime2.Value()];
        }
    }
    return CHIP_NO_ERROR;
}
@end
@implementation MTRGroupKeyManagementClusterKeySetRemoveParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupKeySetID = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGroupKeyManagementClusterKeySetRemoveParams alloc] init];

    other.groupKeySetID = self.groupKeySetID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: groupKeySetID:%@; >", NSStringFromClass([self class]), _groupKeySetID];
    return descriptionString;
}

@end
@implementation MTRGroupKeyManagementClusterKeySetReadAllIndicesParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupKeySetIDs = [NSArray array];
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGroupKeyManagementClusterKeySetReadAllIndicesParams alloc] init];

    other.groupKeySetIDs = self.groupKeySetIDs;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: groupKeySetIDs:%@; >", NSStringFromClass([self class]), _groupKeySetIDs];
    return descriptionString;
}

@end
@implementation MTRGroupKeyManagementClusterKeySetReadAllIndicesResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupKeySetIDs = [NSArray array];
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGroupKeyManagementClusterKeySetReadAllIndicesResponseParams alloc] init];

    other.groupKeySetIDs = self.groupKeySetIDs;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: groupKeySetIDs:%@; >", NSStringFromClass([self class]), _groupKeySetIDs];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::GroupKeyManagement::Commands::KeySetReadAllIndicesResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRGroupKeyManagementClusterKeySetReadAllIndicesResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::GroupKeyManagement::Commands::KeySetReadAllIndicesResponse::DecodableType &)decodableStruct
{
    {
        { // Scope for our temporary variables
            auto * array_0 = [NSMutableArray new];
            auto iter_0 = decodableStruct.groupKeySetIDs.begin();
            while (iter_0.Next()) {
                auto & entry_0 = iter_0.GetValue();
                NSNumber * newElement_0;
                newElement_0 = [NSNumber numberWithUnsignedShort:entry_0];
                [array_0 addObject:newElement_0];
            }
            CHIP_ERROR err = iter_0.GetStatus();
            if (err != CHIP_NO_ERROR) {
                return err;
            }
            self.groupKeySetIDs = array_0;
        }
    }
    return CHIP_NO_ERROR;
}
@end
@implementation MTRModeSelectClusterChangeToModeParams
- (instancetype)init
{
    if (self = [super init]) {

        _newMode = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRModeSelectClusterChangeToModeParams alloc] init];

    other.newMode = self.newMode;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: newMode:%@; >", NSStringFromClass([self class]), _newMode];
    return descriptionString;
}

@end
@implementation MTRTemperatureControlClusterSetTemperatureParams
- (instancetype)init
{
    if (self = [super init]) {

        _targetTemperature = nil;

        _targetTemperatureLevel = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRTemperatureControlClusterSetTemperatureParams alloc] init];

    other.targetTemperature = self.targetTemperature;
    other.targetTemperatureLevel = self.targetTemperatureLevel;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: targetTemperature:%@; targetTemperatureLevel:%@; >",
                                             NSStringFromClass([self class]), _targetTemperature, _targetTemperatureLevel];
    return descriptionString;
}

@end
@implementation MTRRefrigeratorAlarmClusterResetParams
- (instancetype)init
{
    if (self = [super init]) {

        _alarms = @(0);

        _mask = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRRefrigeratorAlarmClusterResetParams alloc] init];

    other.alarms = self.alarms;
    other.mask = self.mask;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: alarms:%@; mask:%@; >", NSStringFromClass([self class]), _alarms, _mask];
    return descriptionString;
}

@end
@implementation MTRSmokeCOAlarmClusterSelfTestRequestParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRSmokeCOAlarmClusterSelfTestRequestParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTROperationalStateClusterPauseParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROperationalStateClusterPauseParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTROperationalStateClusterStopParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROperationalStateClusterStopParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTROperationalStateClusterStartParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROperationalStateClusterStartParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTROperationalStateClusterResumeParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROperationalStateClusterResumeParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTROperationalStateClusterOperationalCommandResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _commandResponseState = [MTROperationalStateClusterErrorStateStruct new];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROperationalStateClusterOperationalCommandResponseParams alloc] init];

    other.commandResponseState = self.commandResponseState;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: commandResponseState:%@; >", NSStringFromClass([self class]), _commandResponseState];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::OperationalState::Commands::OperationalCommandResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTROperationalStateClusterOperationalCommandResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::OperationalState::Commands::OperationalCommandResponse::DecodableType &)decodableStruct
{
    {
        self.commandResponseState = [MTROperationalStateClusterErrorStateStruct new];
        self.commandResponseState.errorStateID =
            [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.commandResponseState.errorStateID)];
        if (decodableStruct.commandResponseState.errorStateLabel.IsNull()) {
            self.commandResponseState.errorStateLabel = nil;
        } else {
            self.commandResponseState.errorStateLabel = AsString(decodableStruct.commandResponseState.errorStateLabel.Value());
            if (self.commandResponseState.errorStateLabel == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                return err;
            }
        }
        if (decodableStruct.commandResponseState.errorStateDetails.HasValue()) {
            self.commandResponseState.errorStateDetails = AsString(decodableStruct.commandResponseState.errorStateDetails.Value());
            if (self.commandResponseState.errorStateDetails == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                return err;
            }
        } else {
            self.commandResponseState.errorStateDetails = nil;
        }
    }
    return CHIP_NO_ERROR;
}
@end
@implementation MTRHEPAFilterMonitoringClusterResetConditionParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRHEPAFilterMonitoringClusterResetConditionParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRActivatedCarbonFilterMonitoringClusterResetConditionParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRActivatedCarbonFilterMonitoringClusterResetConditionParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRCeramicFilterMonitoringClusterResetConditionParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRCeramicFilterMonitoringClusterResetConditionParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRElectrostaticFilterMonitoringClusterResetConditionParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRElectrostaticFilterMonitoringClusterResetConditionParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRUVFilterMonitoringClusterResetConditionParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUVFilterMonitoringClusterResetConditionParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRIonizingFilterMonitoringClusterResetConditionParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRIonizingFilterMonitoringClusterResetConditionParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRZeoliteFilterMonitoringClusterResetConditionParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRZeoliteFilterMonitoringClusterResetConditionParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTROzoneFilterMonitoringClusterResetConditionParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROzoneFilterMonitoringClusterResetConditionParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRWaterTankMonitoringClusterResetConditionParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRWaterTankMonitoringClusterResetConditionParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRFuelTankMonitoringClusterResetConditionParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRFuelTankMonitoringClusterResetConditionParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRInkCartridgeMonitoringClusterResetConditionParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRInkCartridgeMonitoringClusterResetConditionParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRTonerCartridgeMonitoringClusterResetConditionParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRTonerCartridgeMonitoringClusterResetConditionParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRDoorLockClusterLockDoorParams
- (instancetype)init
{
    if (self = [super init]) {

        _pinCode = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterLockDoorParams alloc] init];

    other.pinCode = self.pinCode;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: pinCode:%@; >", NSStringFromClass([self class]), [_pinCode base64EncodedStringWithOptions:0]];
    return descriptionString;
}

@end
@implementation MTRDoorLockClusterUnlockDoorParams
- (instancetype)init
{
    if (self = [super init]) {

        _pinCode = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterUnlockDoorParams alloc] init];

    other.pinCode = self.pinCode;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: pinCode:%@; >", NSStringFromClass([self class]), [_pinCode base64EncodedStringWithOptions:0]];
    return descriptionString;
}

@end
@implementation MTRDoorLockClusterUnlockWithTimeoutParams
- (instancetype)init
{
    if (self = [super init]) {

        _timeout = @(0);

        _pinCode = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterUnlockWithTimeoutParams alloc] init];

    other.timeout = self.timeout;
    other.pinCode = self.pinCode;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: timeout:%@; pinCode:%@; >", NSStringFromClass([self class]),
                                             _timeout, [_pinCode base64EncodedStringWithOptions:0]];
    return descriptionString;
}

@end
@implementation MTRDoorLockClusterSetWeekDayScheduleParams
- (instancetype)init
{
    if (self = [super init]) {

        _weekDayIndex = @(0);

        _userIndex = @(0);

        _daysMask = @(0);

        _startHour = @(0);

        _startMinute = @(0);

        _endHour = @(0);

        _endMinute = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterSetWeekDayScheduleParams alloc] init];

    other.weekDayIndex = self.weekDayIndex;
    other.userIndex = self.userIndex;
    other.daysMask = self.daysMask;
    other.startHour = self.startHour;
    other.startMinute = self.startMinute;
    other.endHour = self.endHour;
    other.endMinute = self.endMinute;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:
            @"<%@: weekDayIndex:%@; userIndex:%@; daysMask:%@; startHour:%@; startMinute:%@; endHour:%@; endMinute:%@; >",
        NSStringFromClass([self class]), _weekDayIndex, _userIndex, _daysMask, _startHour, _startMinute, _endHour, _endMinute];
    return descriptionString;
}

@end
@implementation MTRDoorLockClusterGetWeekDayScheduleParams
- (instancetype)init
{
    if (self = [super init]) {

        _weekDayIndex = @(0);

        _userIndex = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterGetWeekDayScheduleParams alloc] init];

    other.weekDayIndex = self.weekDayIndex;
    other.userIndex = self.userIndex;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: weekDayIndex:%@; userIndex:%@; >", NSStringFromClass([self class]), _weekDayIndex, _userIndex];
    return descriptionString;
}

@end
@implementation MTRDoorLockClusterGetWeekDayScheduleResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _weekDayIndex = @(0);

        _userIndex = @(0);

        _status = @(0);

        _daysMask = nil;

        _startHour = nil;

        _startMinute = nil;

        _endHour = nil;

        _endMinute = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterGetWeekDayScheduleResponseParams alloc] init];

    other.weekDayIndex = self.weekDayIndex;
    other.userIndex = self.userIndex;
    other.status = self.status;
    other.daysMask = self.daysMask;
    other.startHour = self.startHour;
    other.startMinute = self.startMinute;
    other.endHour = self.endHour;
    other.endMinute = self.endMinute;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: weekDayIndex:%@; userIndex:%@; status:%@; daysMask:%@; "
                                                              @"startHour:%@; startMinute:%@; endHour:%@; endMinute:%@; >",
                                             NSStringFromClass([self class]), _weekDayIndex, _userIndex, _status, _daysMask,
                                             _startHour, _startMinute, _endHour, _endMinute];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::DoorLock::Commands::GetWeekDayScheduleResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRDoorLockClusterGetWeekDayScheduleResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::DoorLock::Commands::GetWeekDayScheduleResponse::DecodableType &)decodableStruct
{
    {
        self.weekDayIndex = [NSNumber numberWithUnsignedChar:decodableStruct.weekDayIndex];
    }
    {
        self.userIndex = [NSNumber numberWithUnsignedShort:decodableStruct.userIndex];
    }
    {
        self.status = [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.status)];
    }
    {
        if (decodableStruct.daysMask.HasValue()) {
            self.daysMask = [NSNumber numberWithUnsignedChar:decodableStruct.daysMask.Value().Raw()];
        } else {
            self.daysMask = nil;
        }
    }
    {
        if (decodableStruct.startHour.HasValue()) {
            self.startHour = [NSNumber numberWithUnsignedChar:decodableStruct.startHour.Value()];
        } else {
            self.startHour = nil;
        }
    }
    {
        if (decodableStruct.startMinute.HasValue()) {
            self.startMinute = [NSNumber numberWithUnsignedChar:decodableStruct.startMinute.Value()];
        } else {
            self.startMinute = nil;
        }
    }
    {
        if (decodableStruct.endHour.HasValue()) {
            self.endHour = [NSNumber numberWithUnsignedChar:decodableStruct.endHour.Value()];
        } else {
            self.endHour = nil;
        }
    }
    {
        if (decodableStruct.endMinute.HasValue()) {
            self.endMinute = [NSNumber numberWithUnsignedChar:decodableStruct.endMinute.Value()];
        } else {
            self.endMinute = nil;
        }
    }
    return CHIP_NO_ERROR;
}
@end
@implementation MTRDoorLockClusterClearWeekDayScheduleParams
- (instancetype)init
{
    if (self = [super init]) {

        _weekDayIndex = @(0);

        _userIndex = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterClearWeekDayScheduleParams alloc] init];

    other.weekDayIndex = self.weekDayIndex;
    other.userIndex = self.userIndex;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: weekDayIndex:%@; userIndex:%@; >", NSStringFromClass([self class]), _weekDayIndex, _userIndex];
    return descriptionString;
}

@end
@implementation MTRDoorLockClusterSetYearDayScheduleParams
- (instancetype)init
{
    if (self = [super init]) {

        _yearDayIndex = @(0);

        _userIndex = @(0);

        _localStartTime = @(0);

        _localEndTime = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterSetYearDayScheduleParams alloc] init];

    other.yearDayIndex = self.yearDayIndex;
    other.userIndex = self.userIndex;
    other.localStartTime = self.localStartTime;
    other.localEndTime = self.localEndTime;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: yearDayIndex:%@; userIndex:%@; localStartTime:%@; localEndTime:%@; >",
                  NSStringFromClass([self class]), _yearDayIndex, _userIndex, _localStartTime, _localEndTime];
    return descriptionString;
}

@end
@implementation MTRDoorLockClusterGetYearDayScheduleParams
- (instancetype)init
{
    if (self = [super init]) {

        _yearDayIndex = @(0);

        _userIndex = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterGetYearDayScheduleParams alloc] init];

    other.yearDayIndex = self.yearDayIndex;
    other.userIndex = self.userIndex;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: yearDayIndex:%@; userIndex:%@; >", NSStringFromClass([self class]), _yearDayIndex, _userIndex];
    return descriptionString;
}

@end
@implementation MTRDoorLockClusterGetYearDayScheduleResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _yearDayIndex = @(0);

        _userIndex = @(0);

        _status = @(0);

        _localStartTime = nil;

        _localEndTime = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterGetYearDayScheduleResponseParams alloc] init];

    other.yearDayIndex = self.yearDayIndex;
    other.userIndex = self.userIndex;
    other.status = self.status;
    other.localStartTime = self.localStartTime;
    other.localEndTime = self.localEndTime;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: yearDayIndex:%@; userIndex:%@; status:%@; localStartTime:%@; localEndTime:%@; >",
                  NSStringFromClass([self class]), _yearDayIndex, _userIndex, _status, _localStartTime, _localEndTime];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::DoorLock::Commands::GetYearDayScheduleResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRDoorLockClusterGetYearDayScheduleResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::DoorLock::Commands::GetYearDayScheduleResponse::DecodableType &)decodableStruct
{
    {
        self.yearDayIndex = [NSNumber numberWithUnsignedChar:decodableStruct.yearDayIndex];
    }
    {
        self.userIndex = [NSNumber numberWithUnsignedShort:decodableStruct.userIndex];
    }
    {
        self.status = [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.status)];
    }
    {
        if (decodableStruct.localStartTime.HasValue()) {
            self.localStartTime = [NSNumber numberWithUnsignedInt:decodableStruct.localStartTime.Value()];
        } else {
            self.localStartTime = nil;
        }
    }
    {
        if (decodableStruct.localEndTime.HasValue()) {
            self.localEndTime = [NSNumber numberWithUnsignedInt:decodableStruct.localEndTime.Value()];
        } else {
            self.localEndTime = nil;
        }
    }
    return CHIP_NO_ERROR;
}
@end
@implementation MTRDoorLockClusterClearYearDayScheduleParams
- (instancetype)init
{
    if (self = [super init]) {

        _yearDayIndex = @(0);

        _userIndex = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterClearYearDayScheduleParams alloc] init];

    other.yearDayIndex = self.yearDayIndex;
    other.userIndex = self.userIndex;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: yearDayIndex:%@; userIndex:%@; >", NSStringFromClass([self class]), _yearDayIndex, _userIndex];
    return descriptionString;
}

@end
@implementation MTRDoorLockClusterSetHolidayScheduleParams
- (instancetype)init
{
    if (self = [super init]) {

        _holidayIndex = @(0);

        _localStartTime = @(0);

        _localEndTime = @(0);

        _operatingMode = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterSetHolidayScheduleParams alloc] init];

    other.holidayIndex = self.holidayIndex;
    other.localStartTime = self.localStartTime;
    other.localEndTime = self.localEndTime;
    other.operatingMode = self.operatingMode;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: holidayIndex:%@; localStartTime:%@; localEndTime:%@; operatingMode:%@; >",
                  NSStringFromClass([self class]), _holidayIndex, _localStartTime, _localEndTime, _operatingMode];
    return descriptionString;
}

@end
@implementation MTRDoorLockClusterGetHolidayScheduleParams
- (instancetype)init
{
    if (self = [super init]) {

        _holidayIndex = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterGetHolidayScheduleParams alloc] init];

    other.holidayIndex = self.holidayIndex;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: holidayIndex:%@; >", NSStringFromClass([self class]), _holidayIndex];
    return descriptionString;
}

@end
@implementation MTRDoorLockClusterGetHolidayScheduleResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _holidayIndex = @(0);

        _status = @(0);

        _localStartTime = nil;

        _localEndTime = nil;

        _operatingMode = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterGetHolidayScheduleResponseParams alloc] init];

    other.holidayIndex = self.holidayIndex;
    other.status = self.status;
    other.localStartTime = self.localStartTime;
    other.localEndTime = self.localEndTime;
    other.operatingMode = self.operatingMode;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: holidayIndex:%@; status:%@; localStartTime:%@; localEndTime:%@; operatingMode:%@; >",
                  NSStringFromClass([self class]), _holidayIndex, _status, _localStartTime, _localEndTime, _operatingMode];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::DoorLock::Commands::GetHolidayScheduleResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRDoorLockClusterGetHolidayScheduleResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::DoorLock::Commands::GetHolidayScheduleResponse::DecodableType &)decodableStruct
{
    {
        self.holidayIndex = [NSNumber numberWithUnsignedChar:decodableStruct.holidayIndex];
    }
    {
        self.status = [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.status)];
    }
    {
        if (decodableStruct.localStartTime.HasValue()) {
            self.localStartTime = [NSNumber numberWithUnsignedInt:decodableStruct.localStartTime.Value()];
        } else {
            self.localStartTime = nil;
        }
    }
    {
        if (decodableStruct.localEndTime.HasValue()) {
            self.localEndTime = [NSNumber numberWithUnsignedInt:decodableStruct.localEndTime.Value()];
        } else {
            self.localEndTime = nil;
        }
    }
    {
        if (decodableStruct.operatingMode.HasValue()) {
            self.operatingMode = [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.operatingMode.Value())];
        } else {
            self.operatingMode = nil;
        }
    }
    return CHIP_NO_ERROR;
}
@end
@implementation MTRDoorLockClusterClearHolidayScheduleParams
- (instancetype)init
{
    if (self = [super init]) {

        _holidayIndex = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterClearHolidayScheduleParams alloc] init];

    other.holidayIndex = self.holidayIndex;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: holidayIndex:%@; >", NSStringFromClass([self class]), _holidayIndex];
    return descriptionString;
}

@end
@implementation MTRDoorLockClusterSetUserParams
- (instancetype)init
{
    if (self = [super init]) {

        _operationType = @(0);

        _userIndex = @(0);

        _userName = nil;

        _userUniqueID = nil;

        _userStatus = nil;

        _userType = nil;

        _credentialRule = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterSetUserParams alloc] init];

    other.operationType = self.operationType;
    other.userIndex = self.userIndex;
    other.userName = self.userName;
    other.userUniqueID = self.userUniqueID;
    other.userStatus = self.userStatus;
    other.userType = self.userType;
    other.credentialRule = self.credentialRule;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:
            @"<%@: operationType:%@; userIndex:%@; userName:%@; userUniqueID:%@; userStatus:%@; userType:%@; credentialRule:%@; >",
        NSStringFromClass([self class]), _operationType, _userIndex, _userName, _userUniqueID, _userStatus, _userType,
        _credentialRule];
    return descriptionString;
}

@end

@implementation MTRDoorLockClusterSetUserParams (Deprecated)

- (void)setUserUniqueId:(NSNumber * _Nullable)userUniqueId
{
    self.userUniqueID = userUniqueId;
}

- (NSNumber * _Nullable)userUniqueId
{
    return self.userUniqueID;
}
@end
@implementation MTRDoorLockClusterGetUserParams
- (instancetype)init
{
    if (self = [super init]) {

        _userIndex = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterGetUserParams alloc] init];

    other.userIndex = self.userIndex;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: userIndex:%@; >", NSStringFromClass([self class]), _userIndex];
    return descriptionString;
}

@end
@implementation MTRDoorLockClusterGetUserResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _userIndex = @(0);

        _userName = nil;

        _userUniqueID = nil;

        _userStatus = nil;

        _userType = nil;

        _credentialRule = nil;

        _credentials = nil;

        _creatorFabricIndex = nil;

        _lastModifiedFabricIndex = nil;

        _nextUserIndex = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterGetUserResponseParams alloc] init];

    other.userIndex = self.userIndex;
    other.userName = self.userName;
    other.userUniqueID = self.userUniqueID;
    other.userStatus = self.userStatus;
    other.userType = self.userType;
    other.credentialRule = self.credentialRule;
    other.credentials = self.credentials;
    other.creatorFabricIndex = self.creatorFabricIndex;
    other.lastModifiedFabricIndex = self.lastModifiedFabricIndex;
    other.nextUserIndex = self.nextUserIndex;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: userIndex:%@; userName:%@; userUniqueID:%@; userStatus:%@; userType:%@; credentialRule:%@; "
                         @"credentials:%@; creatorFabricIndex:%@; lastModifiedFabricIndex:%@; nextUserIndex:%@; >",
        NSStringFromClass([self class]), _userIndex, _userName, _userUniqueID, _userStatus, _userType, _credentialRule,
        _credentials, _creatorFabricIndex, _lastModifiedFabricIndex, _nextUserIndex];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::DoorLock::Commands::GetUserResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRDoorLockClusterGetUserResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::DoorLock::Commands::GetUserResponse::DecodableType &)decodableStruct
{
    {
        self.userIndex = [NSNumber numberWithUnsignedShort:decodableStruct.userIndex];
    }
    {
        if (decodableStruct.userName.IsNull()) {
            self.userName = nil;
        } else {
            self.userName = AsString(decodableStruct.userName.Value());
            if (self.userName == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                return err;
            }
        }
    }
    {
        if (decodableStruct.userUniqueID.IsNull()) {
            self.userUniqueID = nil;
        } else {
            self.userUniqueID = [NSNumber numberWithUnsignedInt:decodableStruct.userUniqueID.Value()];
        }
    }
    {
        if (decodableStruct.userStatus.IsNull()) {
            self.userStatus = nil;
        } else {
            self.userStatus = [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.userStatus.Value())];
        }
    }
    {
        if (decodableStruct.userType.IsNull()) {
            self.userType = nil;
        } else {
            self.userType = [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.userType.Value())];
        }
    }
    {
        if (decodableStruct.credentialRule.IsNull()) {
            self.credentialRule = nil;
        } else {
            self.credentialRule = [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.credentialRule.Value())];
        }
    }
    {
        if (decodableStruct.credentials.IsNull()) {
            self.credentials = nil;
        } else {
            { // Scope for our temporary variables
                auto * array_1 = [NSMutableArray new];
                auto iter_1 = decodableStruct.credentials.Value().begin();
                while (iter_1.Next()) {
                    auto & entry_1 = iter_1.GetValue();
                    MTRDoorLockClusterCredentialStruct * newElement_1;
                    newElement_1 = [MTRDoorLockClusterCredentialStruct new];
                    newElement_1.credentialType = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_1.credentialType)];
                    newElement_1.credentialIndex = [NSNumber numberWithUnsignedShort:entry_1.credentialIndex];
                    [array_1 addObject:newElement_1];
                }
                CHIP_ERROR err = iter_1.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    return err;
                }
                self.credentials = array_1;
            }
        }
    }
    {
        if (decodableStruct.creatorFabricIndex.IsNull()) {
            self.creatorFabricIndex = nil;
        } else {
            self.creatorFabricIndex = [NSNumber numberWithUnsignedChar:decodableStruct.creatorFabricIndex.Value()];
        }
    }
    {
        if (decodableStruct.lastModifiedFabricIndex.IsNull()) {
            self.lastModifiedFabricIndex = nil;
        } else {
            self.lastModifiedFabricIndex = [NSNumber numberWithUnsignedChar:decodableStruct.lastModifiedFabricIndex.Value()];
        }
    }
    {
        if (decodableStruct.nextUserIndex.IsNull()) {
            self.nextUserIndex = nil;
        } else {
            self.nextUserIndex = [NSNumber numberWithUnsignedShort:decodableStruct.nextUserIndex.Value()];
        }
    }
    return CHIP_NO_ERROR;
}
@end

@implementation MTRDoorLockClusterGetUserResponseParams (Deprecated)

- (void)setUserUniqueId:(NSNumber * _Nullable)userUniqueId
{
    self.userUniqueID = userUniqueId;
}

- (NSNumber * _Nullable)userUniqueId
{
    return self.userUniqueID;
}
@end
@implementation MTRDoorLockClusterClearUserParams
- (instancetype)init
{
    if (self = [super init]) {

        _userIndex = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterClearUserParams alloc] init];

    other.userIndex = self.userIndex;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: userIndex:%@; >", NSStringFromClass([self class]), _userIndex];
    return descriptionString;
}

@end
@implementation MTRDoorLockClusterSetCredentialParams
- (instancetype)init
{
    if (self = [super init]) {

        _operationType = @(0);

        _credential = [MTRDoorLockClusterCredentialStruct new];

        _credentialData = [NSData data];

        _userIndex = nil;

        _userStatus = nil;

        _userType = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterSetCredentialParams alloc] init];

    other.operationType = self.operationType;
    other.credential = self.credential;
    other.credentialData = self.credentialData;
    other.userIndex = self.userIndex;
    other.userStatus = self.userStatus;
    other.userType = self.userType;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: operationType:%@; credential:%@; credentialData:%@; userIndex:%@; userStatus:%@; userType:%@; >",
        NSStringFromClass([self class]), _operationType, _credential, [_credentialData base64EncodedStringWithOptions:0],
        _userIndex, _userStatus, _userType];
    return descriptionString;
}

@end
@implementation MTRDoorLockClusterSetCredentialResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _userIndex = nil;

        _nextCredentialIndex = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterSetCredentialResponseParams alloc] init];

    other.status = self.status;
    other.userIndex = self.userIndex;
    other.nextCredentialIndex = self.nextCredentialIndex;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: status:%@; userIndex:%@; nextCredentialIndex:%@; >",
                                             NSStringFromClass([self class]), _status, _userIndex, _nextCredentialIndex];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::DoorLock::Commands::SetCredentialResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRDoorLockClusterSetCredentialResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::DoorLock::Commands::SetCredentialResponse::DecodableType &)decodableStruct
{
    {
        self.status = [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.status)];
    }
    {
        if (decodableStruct.userIndex.IsNull()) {
            self.userIndex = nil;
        } else {
            self.userIndex = [NSNumber numberWithUnsignedShort:decodableStruct.userIndex.Value()];
        }
    }
    {
        if (decodableStruct.nextCredentialIndex.IsNull()) {
            self.nextCredentialIndex = nil;
        } else {
            self.nextCredentialIndex = [NSNumber numberWithUnsignedShort:decodableStruct.nextCredentialIndex.Value()];
        }
    }
    return CHIP_NO_ERROR;
}
@end
@implementation MTRDoorLockClusterGetCredentialStatusParams
- (instancetype)init
{
    if (self = [super init]) {

        _credential = [MTRDoorLockClusterCredentialStruct new];
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterGetCredentialStatusParams alloc] init];

    other.credential = self.credential;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: credential:%@; >", NSStringFromClass([self class]), _credential];
    return descriptionString;
}

@end
@implementation MTRDoorLockClusterGetCredentialStatusResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _credentialExists = @(0);

        _userIndex = nil;

        _creatorFabricIndex = nil;

        _lastModifiedFabricIndex = nil;

        _nextCredentialIndex = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterGetCredentialStatusResponseParams alloc] init];

    other.credentialExists = self.credentialExists;
    other.userIndex = self.userIndex;
    other.creatorFabricIndex = self.creatorFabricIndex;
    other.lastModifiedFabricIndex = self.lastModifiedFabricIndex;
    other.nextCredentialIndex = self.nextCredentialIndex;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:
            @"<%@: credentialExists:%@; userIndex:%@; creatorFabricIndex:%@; lastModifiedFabricIndex:%@; nextCredentialIndex:%@; >",
        NSStringFromClass([self class]), _credentialExists, _userIndex, _creatorFabricIndex, _lastModifiedFabricIndex,
        _nextCredentialIndex];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::DoorLock::Commands::GetCredentialStatusResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRDoorLockClusterGetCredentialStatusResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::DoorLock::Commands::GetCredentialStatusResponse::DecodableType &)decodableStruct
{
    {
        self.credentialExists = [NSNumber numberWithBool:decodableStruct.credentialExists];
    }
    {
        if (decodableStruct.userIndex.IsNull()) {
            self.userIndex = nil;
        } else {
            self.userIndex = [NSNumber numberWithUnsignedShort:decodableStruct.userIndex.Value()];
        }
    }
    {
        if (decodableStruct.creatorFabricIndex.IsNull()) {
            self.creatorFabricIndex = nil;
        } else {
            self.creatorFabricIndex = [NSNumber numberWithUnsignedChar:decodableStruct.creatorFabricIndex.Value()];
        }
    }
    {
        if (decodableStruct.lastModifiedFabricIndex.IsNull()) {
            self.lastModifiedFabricIndex = nil;
        } else {
            self.lastModifiedFabricIndex = [NSNumber numberWithUnsignedChar:decodableStruct.lastModifiedFabricIndex.Value()];
        }
    }
    {
        if (decodableStruct.nextCredentialIndex.IsNull()) {
            self.nextCredentialIndex = nil;
        } else {
            self.nextCredentialIndex = [NSNumber numberWithUnsignedShort:decodableStruct.nextCredentialIndex.Value()];
        }
    }
    return CHIP_NO_ERROR;
}
@end
@implementation MTRDoorLockClusterClearCredentialParams
- (instancetype)init
{
    if (self = [super init]) {

        _credential = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterClearCredentialParams alloc] init];

    other.credential = self.credential;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: credential:%@; >", NSStringFromClass([self class]), _credential];
    return descriptionString;
}

@end
@implementation MTRDoorLockClusterUnboltDoorParams
- (instancetype)init
{
    if (self = [super init]) {

        _pinCode = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterUnboltDoorParams alloc] init];

    other.pinCode = self.pinCode;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: pinCode:%@; >", NSStringFromClass([self class]), [_pinCode base64EncodedStringWithOptions:0]];
    return descriptionString;
}

@end
@implementation MTRWindowCoveringClusterUpOrOpenParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRWindowCoveringClusterUpOrOpenParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRWindowCoveringClusterDownOrCloseParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRWindowCoveringClusterDownOrCloseParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRWindowCoveringClusterStopMotionParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRWindowCoveringClusterStopMotionParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRWindowCoveringClusterGoToLiftValueParams
- (instancetype)init
{
    if (self = [super init]) {

        _liftValue = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRWindowCoveringClusterGoToLiftValueParams alloc] init];

    other.liftValue = self.liftValue;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: liftValue:%@; >", NSStringFromClass([self class]), _liftValue];
    return descriptionString;
}

@end
@implementation MTRWindowCoveringClusterGoToLiftPercentageParams
- (instancetype)init
{
    if (self = [super init]) {

        _liftPercent100thsValue = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRWindowCoveringClusterGoToLiftPercentageParams alloc] init];

    other.liftPercent100thsValue = self.liftPercent100thsValue;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: liftPercent100thsValue:%@; >", NSStringFromClass([self class]), _liftPercent100thsValue];
    return descriptionString;
}

@end
@implementation MTRWindowCoveringClusterGoToTiltValueParams
- (instancetype)init
{
    if (self = [super init]) {

        _tiltValue = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRWindowCoveringClusterGoToTiltValueParams alloc] init];

    other.tiltValue = self.tiltValue;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: tiltValue:%@; >", NSStringFromClass([self class]), _tiltValue];
    return descriptionString;
}

@end
@implementation MTRWindowCoveringClusterGoToTiltPercentageParams
- (instancetype)init
{
    if (self = [super init]) {

        _tiltPercent100thsValue = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRWindowCoveringClusterGoToTiltPercentageParams alloc] init];

    other.tiltPercent100thsValue = self.tiltPercent100thsValue;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: tiltPercent100thsValue:%@; >", NSStringFromClass([self class]), _tiltPercent100thsValue];
    return descriptionString;
}

@end
@implementation MTRBarrierControlClusterBarrierControlGoToPercentParams
- (instancetype)init
{
    if (self = [super init]) {

        _percentOpen = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRBarrierControlClusterBarrierControlGoToPercentParams alloc] init];

    other.percentOpen = self.percentOpen;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: percentOpen:%@; >", NSStringFromClass([self class]), _percentOpen];
    return descriptionString;
}

@end
@implementation MTRBarrierControlClusterBarrierControlStopParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRBarrierControlClusterBarrierControlStopParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRThermostatClusterSetpointRaiseLowerParams
- (instancetype)init
{
    if (self = [super init]) {

        _mode = @(0);

        _amount = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRThermostatClusterSetpointRaiseLowerParams alloc] init];

    other.mode = self.mode;
    other.amount = self.amount;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: mode:%@; amount:%@; >", NSStringFromClass([self class]), _mode, _amount];
    return descriptionString;
}

@end
@implementation MTRThermostatClusterGetWeeklyScheduleResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _numberOfTransitionsForSequence = @(0);

        _dayOfWeekForSequence = @(0);

        _modeForSequence = @(0);

        _transitions = [NSArray array];
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRThermostatClusterGetWeeklyScheduleResponseParams alloc] init];

    other.numberOfTransitionsForSequence = self.numberOfTransitionsForSequence;
    other.dayOfWeekForSequence = self.dayOfWeekForSequence;
    other.modeForSequence = self.modeForSequence;
    other.transitions = self.transitions;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: numberOfTransitionsForSequence:%@; dayOfWeekForSequence:%@; modeForSequence:%@; transitions:%@; >",
        NSStringFromClass([self class]), _numberOfTransitionsForSequence, _dayOfWeekForSequence, _modeForSequence, _transitions];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::Thermostat::Commands::GetWeeklyScheduleResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRThermostatClusterGetWeeklyScheduleResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::Thermostat::Commands::GetWeeklyScheduleResponse::DecodableType &)decodableStruct
{
    {
        self.numberOfTransitionsForSequence = [NSNumber numberWithUnsignedChar:decodableStruct.numberOfTransitionsForSequence];
    }
    {
        self.dayOfWeekForSequence = [NSNumber numberWithUnsignedChar:decodableStruct.dayOfWeekForSequence.Raw()];
    }
    {
        self.modeForSequence = [NSNumber numberWithUnsignedChar:decodableStruct.modeForSequence.Raw()];
    }
    {
        { // Scope for our temporary variables
            auto * array_0 = [NSMutableArray new];
            auto iter_0 = decodableStruct.transitions.begin();
            while (iter_0.Next()) {
                auto & entry_0 = iter_0.GetValue();
                MTRThermostatClusterThermostatScheduleTransition * newElement_0;
                newElement_0 = [MTRThermostatClusterThermostatScheduleTransition new];
                newElement_0.transitionTime = [NSNumber numberWithUnsignedShort:entry_0.transitionTime];
                if (entry_0.heatSetpoint.IsNull()) {
                    newElement_0.heatSetpoint = nil;
                } else {
                    newElement_0.heatSetpoint = [NSNumber numberWithShort:entry_0.heatSetpoint.Value()];
                }
                if (entry_0.coolSetpoint.IsNull()) {
                    newElement_0.coolSetpoint = nil;
                } else {
                    newElement_0.coolSetpoint = [NSNumber numberWithShort:entry_0.coolSetpoint.Value()];
                }
                [array_0 addObject:newElement_0];
            }
            CHIP_ERROR err = iter_0.GetStatus();
            if (err != CHIP_NO_ERROR) {
                return err;
            }
            self.transitions = array_0;
        }
    }
    return CHIP_NO_ERROR;
}
@end
@implementation MTRThermostatClusterSetWeeklyScheduleParams
- (instancetype)init
{
    if (self = [super init]) {

        _numberOfTransitionsForSequence = @(0);

        _dayOfWeekForSequence = @(0);

        _modeForSequence = @(0);

        _transitions = [NSArray array];
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRThermostatClusterSetWeeklyScheduleParams alloc] init];

    other.numberOfTransitionsForSequence = self.numberOfTransitionsForSequence;
    other.dayOfWeekForSequence = self.dayOfWeekForSequence;
    other.modeForSequence = self.modeForSequence;
    other.transitions = self.transitions;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: numberOfTransitionsForSequence:%@; dayOfWeekForSequence:%@; modeForSequence:%@; transitions:%@; >",
        NSStringFromClass([self class]), _numberOfTransitionsForSequence, _dayOfWeekForSequence, _modeForSequence, _transitions];
    return descriptionString;
}

@end
@implementation MTRThermostatClusterGetWeeklyScheduleParams
- (instancetype)init
{
    if (self = [super init]) {

        _daysToReturn = @(0);

        _modeToReturn = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRThermostatClusterGetWeeklyScheduleParams alloc] init];

    other.daysToReturn = self.daysToReturn;
    other.modeToReturn = self.modeToReturn;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: daysToReturn:%@; modeToReturn:%@; >",
                                             NSStringFromClass([self class]), _daysToReturn, _modeToReturn];
    return descriptionString;
}

@end
@implementation MTRThermostatClusterClearWeeklyScheduleParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRThermostatClusterClearWeeklyScheduleParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRFanControlClusterStepParams
- (instancetype)init
{
    if (self = [super init]) {

        _direction = @(0);

        _wrap = nil;

        _lowestOff = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRFanControlClusterStepParams alloc] init];

    other.direction = self.direction;
    other.wrap = self.wrap;
    other.lowestOff = self.lowestOff;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: direction:%@; wrap:%@; lowestOff:%@; >",
                                             NSStringFromClass([self class]), _direction, _wrap, _lowestOff];
    return descriptionString;
}

@end
@implementation MTRColorControlClusterMoveToHueParams
- (instancetype)init
{
    if (self = [super init]) {

        _hue = @(0);

        _direction = @(0);

        _transitionTime = @(0);

        _optionsMask = @(0);

        _optionsOverride = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRColorControlClusterMoveToHueParams alloc] init];

    other.hue = self.hue;
    other.direction = self.direction;
    other.transitionTime = self.transitionTime;
    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: hue:%@; direction:%@; transitionTime:%@; optionsMask:%@; optionsOverride:%@; >",
                  NSStringFromClass([self class]), _hue, _direction, _transitionTime, _optionsMask, _optionsOverride];
    return descriptionString;
}

@end
@implementation MTRColorControlClusterMoveHueParams
- (instancetype)init
{
    if (self = [super init]) {

        _moveMode = @(0);

        _rate = @(0);

        _optionsMask = @(0);

        _optionsOverride = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRColorControlClusterMoveHueParams alloc] init];

    other.moveMode = self.moveMode;
    other.rate = self.rate;
    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: moveMode:%@; rate:%@; optionsMask:%@; optionsOverride:%@; >",
                                             NSStringFromClass([self class]), _moveMode, _rate, _optionsMask, _optionsOverride];
    return descriptionString;
}

@end
@implementation MTRColorControlClusterStepHueParams
- (instancetype)init
{
    if (self = [super init]) {

        _stepMode = @(0);

        _stepSize = @(0);

        _transitionTime = @(0);

        _optionsMask = @(0);

        _optionsOverride = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRColorControlClusterStepHueParams alloc] init];

    other.stepMode = self.stepMode;
    other.stepSize = self.stepSize;
    other.transitionTime = self.transitionTime;
    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: stepMode:%@; stepSize:%@; transitionTime:%@; optionsMask:%@; optionsOverride:%@; >",
                  NSStringFromClass([self class]), _stepMode, _stepSize, _transitionTime, _optionsMask, _optionsOverride];
    return descriptionString;
}

@end
@implementation MTRColorControlClusterMoveToSaturationParams
- (instancetype)init
{
    if (self = [super init]) {

        _saturation = @(0);

        _transitionTime = @(0);

        _optionsMask = @(0);

        _optionsOverride = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRColorControlClusterMoveToSaturationParams alloc] init];

    other.saturation = self.saturation;
    other.transitionTime = self.transitionTime;
    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: saturation:%@; transitionTime:%@; optionsMask:%@; optionsOverride:%@; >",
                  NSStringFromClass([self class]), _saturation, _transitionTime, _optionsMask, _optionsOverride];
    return descriptionString;
}

@end
@implementation MTRColorControlClusterMoveSaturationParams
- (instancetype)init
{
    if (self = [super init]) {

        _moveMode = @(0);

        _rate = @(0);

        _optionsMask = @(0);

        _optionsOverride = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRColorControlClusterMoveSaturationParams alloc] init];

    other.moveMode = self.moveMode;
    other.rate = self.rate;
    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: moveMode:%@; rate:%@; optionsMask:%@; optionsOverride:%@; >",
                                             NSStringFromClass([self class]), _moveMode, _rate, _optionsMask, _optionsOverride];
    return descriptionString;
}

@end
@implementation MTRColorControlClusterStepSaturationParams
- (instancetype)init
{
    if (self = [super init]) {

        _stepMode = @(0);

        _stepSize = @(0);

        _transitionTime = @(0);

        _optionsMask = @(0);

        _optionsOverride = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRColorControlClusterStepSaturationParams alloc] init];

    other.stepMode = self.stepMode;
    other.stepSize = self.stepSize;
    other.transitionTime = self.transitionTime;
    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: stepMode:%@; stepSize:%@; transitionTime:%@; optionsMask:%@; optionsOverride:%@; >",
                  NSStringFromClass([self class]), _stepMode, _stepSize, _transitionTime, _optionsMask, _optionsOverride];
    return descriptionString;
}

@end
@implementation MTRColorControlClusterMoveToHueAndSaturationParams
- (instancetype)init
{
    if (self = [super init]) {

        _hue = @(0);

        _saturation = @(0);

        _transitionTime = @(0);

        _optionsMask = @(0);

        _optionsOverride = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRColorControlClusterMoveToHueAndSaturationParams alloc] init];

    other.hue = self.hue;
    other.saturation = self.saturation;
    other.transitionTime = self.transitionTime;
    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: hue:%@; saturation:%@; transitionTime:%@; optionsMask:%@; optionsOverride:%@; >",
                  NSStringFromClass([self class]), _hue, _saturation, _transitionTime, _optionsMask, _optionsOverride];
    return descriptionString;
}

@end
@implementation MTRColorControlClusterMoveToColorParams
- (instancetype)init
{
    if (self = [super init]) {

        _colorX = @(0);

        _colorY = @(0);

        _transitionTime = @(0);

        _optionsMask = @(0);

        _optionsOverride = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRColorControlClusterMoveToColorParams alloc] init];

    other.colorX = self.colorX;
    other.colorY = self.colorY;
    other.transitionTime = self.transitionTime;
    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: colorX:%@; colorY:%@; transitionTime:%@; optionsMask:%@; optionsOverride:%@; >",
                  NSStringFromClass([self class]), _colorX, _colorY, _transitionTime, _optionsMask, _optionsOverride];
    return descriptionString;
}

@end
@implementation MTRColorControlClusterMoveColorParams
- (instancetype)init
{
    if (self = [super init]) {

        _rateX = @(0);

        _rateY = @(0);

        _optionsMask = @(0);

        _optionsOverride = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRColorControlClusterMoveColorParams alloc] init];

    other.rateX = self.rateX;
    other.rateY = self.rateY;
    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: rateX:%@; rateY:%@; optionsMask:%@; optionsOverride:%@; >",
                                             NSStringFromClass([self class]), _rateX, _rateY, _optionsMask, _optionsOverride];
    return descriptionString;
}

@end
@implementation MTRColorControlClusterStepColorParams
- (instancetype)init
{
    if (self = [super init]) {

        _stepX = @(0);

        _stepY = @(0);

        _transitionTime = @(0);

        _optionsMask = @(0);

        _optionsOverride = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRColorControlClusterStepColorParams alloc] init];

    other.stepX = self.stepX;
    other.stepY = self.stepY;
    other.transitionTime = self.transitionTime;
    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: stepX:%@; stepY:%@; transitionTime:%@; optionsMask:%@; optionsOverride:%@; >",
                  NSStringFromClass([self class]), _stepX, _stepY, _transitionTime, _optionsMask, _optionsOverride];
    return descriptionString;
}

@end
@implementation MTRColorControlClusterMoveToColorTemperatureParams
- (instancetype)init
{
    if (self = [super init]) {

        _colorTemperatureMireds = @(0);

        _transitionTime = @(0);

        _optionsMask = @(0);

        _optionsOverride = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRColorControlClusterMoveToColorTemperatureParams alloc] init];

    other.colorTemperatureMireds = self.colorTemperatureMireds;
    other.transitionTime = self.transitionTime;
    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: colorTemperatureMireds:%@; transitionTime:%@; optionsMask:%@; optionsOverride:%@; >",
                  NSStringFromClass([self class]), _colorTemperatureMireds, _transitionTime, _optionsMask, _optionsOverride];
    return descriptionString;
}

@end

@implementation MTRColorControlClusterMoveToColorTemperatureParams (Deprecated)

- (void)setColorTemperature:(NSNumber * _Nonnull)colorTemperature
{
    self.colorTemperatureMireds = colorTemperature;
}

- (NSNumber * _Nonnull)colorTemperature
{
    return self.colorTemperatureMireds;
}
@end
@implementation MTRColorControlClusterEnhancedMoveToHueParams
- (instancetype)init
{
    if (self = [super init]) {

        _enhancedHue = @(0);

        _direction = @(0);

        _transitionTime = @(0);

        _optionsMask = @(0);

        _optionsOverride = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRColorControlClusterEnhancedMoveToHueParams alloc] init];

    other.enhancedHue = self.enhancedHue;
    other.direction = self.direction;
    other.transitionTime = self.transitionTime;
    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: enhancedHue:%@; direction:%@; transitionTime:%@; optionsMask:%@; optionsOverride:%@; >",
                  NSStringFromClass([self class]), _enhancedHue, _direction, _transitionTime, _optionsMask, _optionsOverride];
    return descriptionString;
}

@end
@implementation MTRColorControlClusterEnhancedMoveHueParams
- (instancetype)init
{
    if (self = [super init]) {

        _moveMode = @(0);

        _rate = @(0);

        _optionsMask = @(0);

        _optionsOverride = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRColorControlClusterEnhancedMoveHueParams alloc] init];

    other.moveMode = self.moveMode;
    other.rate = self.rate;
    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: moveMode:%@; rate:%@; optionsMask:%@; optionsOverride:%@; >",
                                             NSStringFromClass([self class]), _moveMode, _rate, _optionsMask, _optionsOverride];
    return descriptionString;
}

@end
@implementation MTRColorControlClusterEnhancedStepHueParams
- (instancetype)init
{
    if (self = [super init]) {

        _stepMode = @(0);

        _stepSize = @(0);

        _transitionTime = @(0);

        _optionsMask = @(0);

        _optionsOverride = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRColorControlClusterEnhancedStepHueParams alloc] init];

    other.stepMode = self.stepMode;
    other.stepSize = self.stepSize;
    other.transitionTime = self.transitionTime;
    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: stepMode:%@; stepSize:%@; transitionTime:%@; optionsMask:%@; optionsOverride:%@; >",
                  NSStringFromClass([self class]), _stepMode, _stepSize, _transitionTime, _optionsMask, _optionsOverride];
    return descriptionString;
}

@end
@implementation MTRColorControlClusterEnhancedMoveToHueAndSaturationParams
- (instancetype)init
{
    if (self = [super init]) {

        _enhancedHue = @(0);

        _saturation = @(0);

        _transitionTime = @(0);

        _optionsMask = @(0);

        _optionsOverride = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRColorControlClusterEnhancedMoveToHueAndSaturationParams alloc] init];

    other.enhancedHue = self.enhancedHue;
    other.saturation = self.saturation;
    other.transitionTime = self.transitionTime;
    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: enhancedHue:%@; saturation:%@; transitionTime:%@; optionsMask:%@; optionsOverride:%@; >",
                  NSStringFromClass([self class]), _enhancedHue, _saturation, _transitionTime, _optionsMask, _optionsOverride];
    return descriptionString;
}

@end
@implementation MTRColorControlClusterColorLoopSetParams
- (instancetype)init
{
    if (self = [super init]) {

        _updateFlags = @(0);

        _action = @(0);

        _direction = @(0);

        _time = @(0);

        _startHue = @(0);

        _optionsMask = @(0);

        _optionsOverride = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRColorControlClusterColorLoopSetParams alloc] init];

    other.updateFlags = self.updateFlags;
    other.action = self.action;
    other.direction = self.direction;
    other.time = self.time;
    other.startHue = self.startHue;
    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:
            @"<%@: updateFlags:%@; action:%@; direction:%@; time:%@; startHue:%@; optionsMask:%@; optionsOverride:%@; >",
        NSStringFromClass([self class]), _updateFlags, _action, _direction, _time, _startHue, _optionsMask, _optionsOverride];
    return descriptionString;
}

@end
@implementation MTRColorControlClusterStopMoveStepParams
- (instancetype)init
{
    if (self = [super init]) {

        _optionsMask = @(0);

        _optionsOverride = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRColorControlClusterStopMoveStepParams alloc] init];

    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: optionsMask:%@; optionsOverride:%@; >",
                                             NSStringFromClass([self class]), _optionsMask, _optionsOverride];
    return descriptionString;
}

@end
@implementation MTRColorControlClusterMoveColorTemperatureParams
- (instancetype)init
{
    if (self = [super init]) {

        _moveMode = @(0);

        _rate = @(0);

        _colorTemperatureMinimumMireds = @(0);

        _colorTemperatureMaximumMireds = @(0);

        _optionsMask = @(0);

        _optionsOverride = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRColorControlClusterMoveColorTemperatureParams alloc] init];

    other.moveMode = self.moveMode;
    other.rate = self.rate;
    other.colorTemperatureMinimumMireds = self.colorTemperatureMinimumMireds;
    other.colorTemperatureMaximumMireds = self.colorTemperatureMaximumMireds;
    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: moveMode:%@; rate:%@; colorTemperatureMinimumMireds:%@; "
                                   @"colorTemperatureMaximumMireds:%@; optionsMask:%@; optionsOverride:%@; >",
                  NSStringFromClass([self class]), _moveMode, _rate, _colorTemperatureMinimumMireds, _colorTemperatureMaximumMireds,
                  _optionsMask, _optionsOverride];
    return descriptionString;
}

@end
@implementation MTRColorControlClusterStepColorTemperatureParams
- (instancetype)init
{
    if (self = [super init]) {

        _stepMode = @(0);

        _stepSize = @(0);

        _transitionTime = @(0);

        _colorTemperatureMinimumMireds = @(0);

        _colorTemperatureMaximumMireds = @(0);

        _optionsMask = @(0);

        _optionsOverride = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRColorControlClusterStepColorTemperatureParams alloc] init];

    other.stepMode = self.stepMode;
    other.stepSize = self.stepSize;
    other.transitionTime = self.transitionTime;
    other.colorTemperatureMinimumMireds = self.colorTemperatureMinimumMireds;
    other.colorTemperatureMaximumMireds = self.colorTemperatureMaximumMireds;
    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: stepMode:%@; stepSize:%@; transitionTime:%@; colorTemperatureMinimumMireds:%@; "
                                   @"colorTemperatureMaximumMireds:%@; optionsMask:%@; optionsOverride:%@; >",
                  NSStringFromClass([self class]), _stepMode, _stepSize, _transitionTime, _colorTemperatureMinimumMireds,
                  _colorTemperatureMaximumMireds, _optionsMask, _optionsOverride];
    return descriptionString;
}

@end
@implementation MTRChannelClusterChangeChannelParams
- (instancetype)init
{
    if (self = [super init]) {

        _match = @"";
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRChannelClusterChangeChannelParams alloc] init];

    other.match = self.match;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: match:%@; >", NSStringFromClass([self class]), _match];
    return descriptionString;
}

@end
@implementation MTRChannelClusterChangeChannelResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _data = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRChannelClusterChangeChannelResponseParams alloc] init];

    other.status = self.status;
    other.data = self.data;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: status:%@; data:%@; >", NSStringFromClass([self class]), _status, _data];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::Channel::Commands::ChangeChannelResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRChannelClusterChangeChannelResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::Channel::Commands::ChangeChannelResponse::DecodableType &)decodableStruct
{
    {
        self.status = [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.status)];
    }
    {
        if (decodableStruct.data.HasValue()) {
            self.data = AsString(decodableStruct.data.Value());
            if (self.data == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                return err;
            }
        } else {
            self.data = nil;
        }
    }
    return CHIP_NO_ERROR;
}
@end
@implementation MTRChannelClusterChangeChannelByNumberParams
- (instancetype)init
{
    if (self = [super init]) {

        _majorNumber = @(0);

        _minorNumber = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRChannelClusterChangeChannelByNumberParams alloc] init];

    other.majorNumber = self.majorNumber;
    other.minorNumber = self.minorNumber;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: majorNumber:%@; minorNumber:%@; >", NSStringFromClass([self class]), _majorNumber, _minorNumber];
    return descriptionString;
}

@end
@implementation MTRChannelClusterSkipChannelParams
- (instancetype)init
{
    if (self = [super init]) {

        _count = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRChannelClusterSkipChannelParams alloc] init];

    other.count = self.count;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: count:%@; >", NSStringFromClass([self class]), _count];
    return descriptionString;
}

@end
@implementation MTRTargetNavigatorClusterNavigateTargetParams
- (instancetype)init
{
    if (self = [super init]) {

        _target = @(0);

        _data = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRTargetNavigatorClusterNavigateTargetParams alloc] init];

    other.target = self.target;
    other.data = self.data;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: target:%@; data:%@; >", NSStringFromClass([self class]), _target, _data];
    return descriptionString;
}

@end
@implementation MTRTargetNavigatorClusterNavigateTargetResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _data = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRTargetNavigatorClusterNavigateTargetResponseParams alloc] init];

    other.status = self.status;
    other.data = self.data;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: status:%@; data:%@; >", NSStringFromClass([self class]), _status, _data];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::TargetNavigator::Commands::NavigateTargetResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRTargetNavigatorClusterNavigateTargetResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::TargetNavigator::Commands::NavigateTargetResponse::DecodableType &)decodableStruct
{
    {
        self.status = [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.status)];
    }
    {
        if (decodableStruct.data.HasValue()) {
            self.data = AsString(decodableStruct.data.Value());
            if (self.data == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                return err;
            }
        } else {
            self.data = nil;
        }
    }
    return CHIP_NO_ERROR;
}
@end
@implementation MTRMediaPlaybackClusterPlayParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaPlaybackClusterPlayParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRMediaPlaybackClusterPauseParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaPlaybackClusterPauseParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRMediaPlaybackClusterStopParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaPlaybackClusterStopParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRMediaPlaybackClusterStopPlaybackParams
@end
@implementation MTRMediaPlaybackClusterStartOverParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaPlaybackClusterStartOverParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRMediaPlaybackClusterPreviousParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaPlaybackClusterPreviousParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRMediaPlaybackClusterNextParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaPlaybackClusterNextParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRMediaPlaybackClusterRewindParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaPlaybackClusterRewindParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRMediaPlaybackClusterFastForwardParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaPlaybackClusterFastForwardParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRMediaPlaybackClusterSkipForwardParams
- (instancetype)init
{
    if (self = [super init]) {

        _deltaPositionMilliseconds = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaPlaybackClusterSkipForwardParams alloc] init];

    other.deltaPositionMilliseconds = self.deltaPositionMilliseconds;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: deltaPositionMilliseconds:%@; >", NSStringFromClass([self class]), _deltaPositionMilliseconds];
    return descriptionString;
}

@end
@implementation MTRMediaPlaybackClusterSkipBackwardParams
- (instancetype)init
{
    if (self = [super init]) {

        _deltaPositionMilliseconds = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaPlaybackClusterSkipBackwardParams alloc] init];

    other.deltaPositionMilliseconds = self.deltaPositionMilliseconds;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: deltaPositionMilliseconds:%@; >", NSStringFromClass([self class]), _deltaPositionMilliseconds];
    return descriptionString;
}

@end
@implementation MTRMediaPlaybackClusterPlaybackResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _data = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaPlaybackClusterPlaybackResponseParams alloc] init];

    other.status = self.status;
    other.data = self.data;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: status:%@; data:%@; >", NSStringFromClass([self class]), _status, _data];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRMediaPlaybackClusterPlaybackResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::DecodableType &)decodableStruct
{
    {
        self.status = [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.status)];
    }
    {
        if (decodableStruct.data.HasValue()) {
            self.data = AsString(decodableStruct.data.Value());
            if (self.data == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                return err;
            }
        } else {
            self.data = nil;
        }
    }
    return CHIP_NO_ERROR;
}
@end
@implementation MTRMediaPlaybackClusterSeekParams
- (instancetype)init
{
    if (self = [super init]) {

        _position = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaPlaybackClusterSeekParams alloc] init];

    other.position = self.position;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: position:%@; >", NSStringFromClass([self class]), _position];
    return descriptionString;
}

@end
@implementation MTRMediaInputClusterSelectInputParams
- (instancetype)init
{
    if (self = [super init]) {

        _index = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaInputClusterSelectInputParams alloc] init];

    other.index = self.index;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: index:%@; >", NSStringFromClass([self class]), _index];
    return descriptionString;
}

@end
@implementation MTRMediaInputClusterShowInputStatusParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaInputClusterShowInputStatusParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRMediaInputClusterHideInputStatusParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaInputClusterHideInputStatusParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRMediaInputClusterRenameInputParams
- (instancetype)init
{
    if (self = [super init]) {

        _index = @(0);

        _name = @"";
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaInputClusterRenameInputParams alloc] init];

    other.index = self.index;
    other.name = self.name;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: index:%@; name:%@; >", NSStringFromClass([self class]), _index, _name];
    return descriptionString;
}

@end
@implementation MTRLowPowerClusterSleepParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRLowPowerClusterSleepParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRKeypadInputClusterSendKeyParams
- (instancetype)init
{
    if (self = [super init]) {

        _keyCode = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRKeypadInputClusterSendKeyParams alloc] init];

    other.keyCode = self.keyCode;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: keyCode:%@; >", NSStringFromClass([self class]), _keyCode];
    return descriptionString;
}

@end
@implementation MTRKeypadInputClusterSendKeyResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRKeypadInputClusterSendKeyResponseParams alloc] init];

    other.status = self.status;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: status:%@; >", NSStringFromClass([self class]), _status];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::KeypadInput::Commands::SendKeyResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRKeypadInputClusterSendKeyResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::KeypadInput::Commands::SendKeyResponse::DecodableType &)decodableStruct
{
    {
        self.status = [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.status)];
    }
    return CHIP_NO_ERROR;
}
@end
@implementation MTRContentLauncherClusterLaunchContentParams
- (instancetype)init
{
    if (self = [super init]) {

        _search = [MTRContentLauncherClusterContentSearchStruct new];

        _autoPlay = @(0);

        _data = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRContentLauncherClusterLaunchContentParams alloc] init];

    other.search = self.search;
    other.autoPlay = self.autoPlay;
    other.data = self.data;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: search:%@; autoPlay:%@; data:%@; >", NSStringFromClass([self class]), _search, _autoPlay, _data];
    return descriptionString;
}

@end
@implementation MTRContentLauncherClusterLaunchURLParams
- (instancetype)init
{
    if (self = [super init]) {

        _contentURL = @"";

        _displayString = nil;

        _brandingInformation = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRContentLauncherClusterLaunchURLParams alloc] init];

    other.contentURL = self.contentURL;
    other.displayString = self.displayString;
    other.brandingInformation = self.brandingInformation;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: contentURL:%@; displayString:%@; brandingInformation:%@; >",
                                             NSStringFromClass([self class]), _contentURL, _displayString, _brandingInformation];
    return descriptionString;
}

@end
@implementation MTRContentLauncherClusterLauncherResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _data = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRContentLauncherClusterLauncherResponseParams alloc] init];

    other.status = self.status;
    other.data = self.data;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: status:%@; data:%@; >", NSStringFromClass([self class]), _status, _data];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::ContentLauncher::Commands::LauncherResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRContentLauncherClusterLauncherResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::ContentLauncher::Commands::LauncherResponse::DecodableType &)decodableStruct
{
    {
        self.status = [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.status)];
    }
    {
        if (decodableStruct.data.HasValue()) {
            self.data = AsString(decodableStruct.data.Value());
            if (self.data == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                return err;
            }
        } else {
            self.data = nil;
        }
    }
    return CHIP_NO_ERROR;
}
@end

@implementation MTRContentLauncherClusterLaunchResponseParams
@end
@implementation MTRAudioOutputClusterSelectOutputParams
- (instancetype)init
{
    if (self = [super init]) {

        _index = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRAudioOutputClusterSelectOutputParams alloc] init];

    other.index = self.index;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: index:%@; >", NSStringFromClass([self class]), _index];
    return descriptionString;
}

@end
@implementation MTRAudioOutputClusterRenameOutputParams
- (instancetype)init
{
    if (self = [super init]) {

        _index = @(0);

        _name = @"";
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRAudioOutputClusterRenameOutputParams alloc] init];

    other.index = self.index;
    other.name = self.name;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: index:%@; name:%@; >", NSStringFromClass([self class]), _index, _name];
    return descriptionString;
}

@end
@implementation MTRApplicationLauncherClusterLaunchAppParams
- (instancetype)init
{
    if (self = [super init]) {

        _application = nil;

        _data = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRApplicationLauncherClusterLaunchAppParams alloc] init];

    other.application = self.application;
    other.data = self.data;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: application:%@; data:%@; >", NSStringFromClass([self class]),
                                             _application, [_data base64EncodedStringWithOptions:0]];
    return descriptionString;
}

@end
@implementation MTRApplicationLauncherClusterStopAppParams
- (instancetype)init
{
    if (self = [super init]) {

        _application = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRApplicationLauncherClusterStopAppParams alloc] init];

    other.application = self.application;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: application:%@; >", NSStringFromClass([self class]), _application];
    return descriptionString;
}

@end
@implementation MTRApplicationLauncherClusterHideAppParams
- (instancetype)init
{
    if (self = [super init]) {

        _application = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRApplicationLauncherClusterHideAppParams alloc] init];

    other.application = self.application;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: application:%@; >", NSStringFromClass([self class]), _application];
    return descriptionString;
}

@end
@implementation MTRApplicationLauncherClusterLauncherResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _data = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRApplicationLauncherClusterLauncherResponseParams alloc] init];

    other.status = self.status;
    other.data = self.data;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: status:%@; data:%@; >", NSStringFromClass([self class]),
                                             _status, [_data base64EncodedStringWithOptions:0]];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::ApplicationLauncher::Commands::LauncherResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRApplicationLauncherClusterLauncherResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::ApplicationLauncher::Commands::LauncherResponse::DecodableType &)decodableStruct
{
    {
        self.status = [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.status)];
    }
    {
        if (decodableStruct.data.HasValue()) {
            self.data = AsData(decodableStruct.data.Value());
        } else {
            self.data = nil;
        }
    }
    return CHIP_NO_ERROR;
}
@end
@implementation MTRAccountLoginClusterGetSetupPINParams
- (instancetype)init
{
    if (self = [super init]) {

        _tempAccountIdentifier = @"";
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRAccountLoginClusterGetSetupPINParams alloc] init];

    other.tempAccountIdentifier = self.tempAccountIdentifier;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: tempAccountIdentifier:%@; >", NSStringFromClass([self class]), _tempAccountIdentifier];
    return descriptionString;
}

@end
@implementation MTRAccountLoginClusterGetSetupPINResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _setupPIN = @"";
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRAccountLoginClusterGetSetupPINResponseParams alloc] init];

    other.setupPIN = self.setupPIN;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: setupPIN:%@; >", NSStringFromClass([self class]), _setupPIN];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::AccountLogin::Commands::GetSetupPINResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRAccountLoginClusterGetSetupPINResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::AccountLogin::Commands::GetSetupPINResponse::DecodableType &)decodableStruct
{
    {
        self.setupPIN = AsString(decodableStruct.setupPIN);
        if (self.setupPIN == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            return err;
        }
    }
    return CHIP_NO_ERROR;
}
@end
@implementation MTRAccountLoginClusterLoginParams
- (instancetype)init
{
    if (self = [super init]) {

        _tempAccountIdentifier = @"";

        _setupPIN = @"";
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRAccountLoginClusterLoginParams alloc] init];

    other.tempAccountIdentifier = self.tempAccountIdentifier;
    other.setupPIN = self.setupPIN;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: tempAccountIdentifier:%@; setupPIN:%@; >",
                                             NSStringFromClass([self class]), _tempAccountIdentifier, _setupPIN];
    return descriptionString;
}

@end
@implementation MTRAccountLoginClusterLogoutParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRAccountLoginClusterLogoutParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRElectricalMeasurementClusterGetProfileInfoResponseCommandParams
- (instancetype)init
{
    if (self = [super init]) {

        _profileCount = @(0);

        _profileIntervalPeriod = @(0);

        _maxNumberOfIntervals = @(0);

        _listOfAttributes = [NSArray array];
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRElectricalMeasurementClusterGetProfileInfoResponseCommandParams alloc] init];

    other.profileCount = self.profileCount;
    other.profileIntervalPeriod = self.profileIntervalPeriod;
    other.maxNumberOfIntervals = self.maxNumberOfIntervals;
    other.listOfAttributes = self.listOfAttributes;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: profileCount:%@; profileIntervalPeriod:%@; maxNumberOfIntervals:%@; listOfAttributes:%@; >",
        NSStringFromClass([self class]), _profileCount, _profileIntervalPeriod, _maxNumberOfIntervals, _listOfAttributes];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::ElectricalMeasurement::Commands::GetProfileInfoResponseCommand::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRElectricalMeasurementClusterGetProfileInfoResponseCommandParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::ElectricalMeasurement::Commands::GetProfileInfoResponseCommand::DecodableType &)decodableStruct
{
    {
        self.profileCount = [NSNumber numberWithUnsignedChar:decodableStruct.profileCount];
    }
    {
        self.profileIntervalPeriod = [NSNumber numberWithUnsignedChar:decodableStruct.profileIntervalPeriod];
    }
    {
        self.maxNumberOfIntervals = [NSNumber numberWithUnsignedChar:decodableStruct.maxNumberOfIntervals];
    }
    {
        { // Scope for our temporary variables
            auto * array_0 = [NSMutableArray new];
            auto iter_0 = decodableStruct.listOfAttributes.begin();
            while (iter_0.Next()) {
                auto & entry_0 = iter_0.GetValue();
                NSNumber * newElement_0;
                newElement_0 = [NSNumber numberWithUnsignedShort:entry_0];
                [array_0 addObject:newElement_0];
            }
            CHIP_ERROR err = iter_0.GetStatus();
            if (err != CHIP_NO_ERROR) {
                return err;
            }
            self.listOfAttributes = array_0;
        }
    }
    return CHIP_NO_ERROR;
}
@end
@implementation MTRElectricalMeasurementClusterGetProfileInfoCommandParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRElectricalMeasurementClusterGetProfileInfoCommandParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRElectricalMeasurementClusterGetMeasurementProfileResponseCommandParams
- (instancetype)init
{
    if (self = [super init]) {

        _startTime = @(0);

        _status = @(0);

        _profileIntervalPeriod = @(0);

        _numberOfIntervalsDelivered = @(0);

        _attributeId = @(0);

        _intervals = [NSArray array];
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRElectricalMeasurementClusterGetMeasurementProfileResponseCommandParams alloc] init];

    other.startTime = self.startTime;
    other.status = self.status;
    other.profileIntervalPeriod = self.profileIntervalPeriod;
    other.numberOfIntervalsDelivered = self.numberOfIntervalsDelivered;
    other.attributeId = self.attributeId;
    other.intervals = self.intervals;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: startTime:%@; status:%@; profileIntervalPeriod:%@; "
                                                              @"numberOfIntervalsDelivered:%@; attributeId:%@; intervals:%@; >",
                                             NSStringFromClass([self class]), _startTime, _status, _profileIntervalPeriod,
                                             _numberOfIntervalsDelivered, _attributeId, _intervals];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::ElectricalMeasurement::Commands::GetMeasurementProfileResponseCommand::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRElectricalMeasurementClusterGetMeasurementProfileResponseCommandParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::ElectricalMeasurement::Commands::GetMeasurementProfileResponseCommand::DecodableType &)
        decodableStruct
{
    {
        self.startTime = [NSNumber numberWithUnsignedInt:decodableStruct.startTime];
    }
    {
        self.status = [NSNumber numberWithUnsignedChar:decodableStruct.status];
    }
    {
        self.profileIntervalPeriod = [NSNumber numberWithUnsignedChar:decodableStruct.profileIntervalPeriod];
    }
    {
        self.numberOfIntervalsDelivered = [NSNumber numberWithUnsignedChar:decodableStruct.numberOfIntervalsDelivered];
    }
    {
        self.attributeId = [NSNumber numberWithUnsignedShort:decodableStruct.attributeId];
    }
    {
        { // Scope for our temporary variables
            auto * array_0 = [NSMutableArray new];
            auto iter_0 = decodableStruct.intervals.begin();
            while (iter_0.Next()) {
                auto & entry_0 = iter_0.GetValue();
                NSNumber * newElement_0;
                newElement_0 = [NSNumber numberWithUnsignedChar:entry_0];
                [array_0 addObject:newElement_0];
            }
            CHIP_ERROR err = iter_0.GetStatus();
            if (err != CHIP_NO_ERROR) {
                return err;
            }
            self.intervals = array_0;
        }
    }
    return CHIP_NO_ERROR;
}
@end
@implementation MTRElectricalMeasurementClusterGetMeasurementProfileCommandParams
- (instancetype)init
{
    if (self = [super init]) {

        _attributeId = @(0);

        _startTime = @(0);

        _numberOfIntervals = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRElectricalMeasurementClusterGetMeasurementProfileCommandParams alloc] init];

    other.attributeId = self.attributeId;
    other.startTime = self.startTime;
    other.numberOfIntervals = self.numberOfIntervals;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: attributeId:%@; startTime:%@; numberOfIntervals:%@; >",
                                             NSStringFromClass([self class]), _attributeId, _startTime, _numberOfIntervals];
    return descriptionString;
}

@end
@implementation MTRUnitTestingClusterTestParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTestParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterTestParams
@end
@implementation MTRUnitTestingClusterTestSpecificResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _returnValue = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTestSpecificResponseParams alloc] init];

    other.returnValue = self.returnValue;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: returnValue:%@; >", NSStringFromClass([self class]), _returnValue];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::UnitTesting::Commands::TestSpecificResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRUnitTestingClusterTestSpecificResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::UnitTesting::Commands::TestSpecificResponse::DecodableType &)decodableStruct
{
    {
        self.returnValue = [NSNumber numberWithUnsignedChar:decodableStruct.returnValue];
    }
    return CHIP_NO_ERROR;
}
@end

@implementation MTRTestClusterClusterTestSpecificResponseParams
@end
@implementation MTRUnitTestingClusterTestNotHandledParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTestNotHandledParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterTestNotHandledParams
@end
@implementation MTRUnitTestingClusterTestAddArgumentsResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _returnValue = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTestAddArgumentsResponseParams alloc] init];

    other.returnValue = self.returnValue;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: returnValue:%@; >", NSStringFromClass([self class]), _returnValue];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::UnitTesting::Commands::TestAddArgumentsResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRUnitTestingClusterTestAddArgumentsResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::UnitTesting::Commands::TestAddArgumentsResponse::DecodableType &)decodableStruct
{
    {
        self.returnValue = [NSNumber numberWithUnsignedChar:decodableStruct.returnValue];
    }
    return CHIP_NO_ERROR;
}
@end

@implementation MTRTestClusterClusterTestAddArgumentsResponseParams
@end
@implementation MTRUnitTestingClusterTestSpecificParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTestSpecificParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterTestSpecificParams
@end
@implementation MTRUnitTestingClusterTestSimpleArgumentResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _returnValue = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTestSimpleArgumentResponseParams alloc] init];

    other.returnValue = self.returnValue;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: returnValue:%@; >", NSStringFromClass([self class]), _returnValue];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::UnitTesting::Commands::TestSimpleArgumentResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRUnitTestingClusterTestSimpleArgumentResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::UnitTesting::Commands::TestSimpleArgumentResponse::DecodableType &)decodableStruct
{
    {
        self.returnValue = [NSNumber numberWithBool:decodableStruct.returnValue];
    }
    return CHIP_NO_ERROR;
}
@end

@implementation MTRTestClusterClusterTestSimpleArgumentResponseParams
@end
@implementation MTRUnitTestingClusterTestUnknownCommandParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTestUnknownCommandParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterTestUnknownCommandParams
@end
@implementation MTRUnitTestingClusterTestStructArrayArgumentResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = [NSArray array];

        _arg2 = [NSArray array];

        _arg3 = [NSArray array];

        _arg4 = [NSArray array];

        _arg5 = @(0);

        _arg6 = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTestStructArrayArgumentResponseParams alloc] init];

    other.arg1 = self.arg1;
    other.arg2 = self.arg2;
    other.arg3 = self.arg3;
    other.arg4 = self.arg4;
    other.arg5 = self.arg5;
    other.arg6 = self.arg6;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; arg2:%@; arg3:%@; arg4:%@; arg5:%@; arg6:%@; >",
                                             NSStringFromClass([self class]), _arg1, _arg2, _arg3, _arg4, _arg5, _arg6];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::UnitTesting::Commands::TestStructArrayArgumentResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRUnitTestingClusterTestStructArrayArgumentResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::UnitTesting::Commands::TestStructArrayArgumentResponse::DecodableType &)decodableStruct
{
    { { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
    auto iter_0 = decodableStruct.arg1.begin();
    while (iter_0.Next()) {
        auto & entry_0 = iter_0.GetValue();
        MTRUnitTestingClusterNestedStructList * newElement_0;
        newElement_0 = [MTRUnitTestingClusterNestedStructList new];
        newElement_0.a = [NSNumber numberWithUnsignedChar:entry_0.a];
        newElement_0.b = [NSNumber numberWithBool:entry_0.b];
        newElement_0.c = [MTRUnitTestingClusterSimpleStruct new];
        newElement_0.c.a = [NSNumber numberWithUnsignedChar:entry_0.c.a];
        newElement_0.c.b = [NSNumber numberWithBool:entry_0.c.b];
        newElement_0.c.c = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.c.c)];
        newElement_0.c.d = AsData(entry_0.c.d);
        newElement_0.c.e = AsString(entry_0.c.e);
        if (newElement_0.c.e == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            return err;
        }
        newElement_0.c.f = [NSNumber numberWithUnsignedChar:entry_0.c.f.Raw()];
        newElement_0.c.g = [NSNumber numberWithFloat:entry_0.c.g];
        newElement_0.c.h = [NSNumber numberWithDouble:entry_0.c.h];
        { // Scope for our temporary variables
            auto * array_2 = [NSMutableArray new];
            auto iter_2 = entry_0.d.begin();
            while (iter_2.Next()) {
                auto & entry_2 = iter_2.GetValue();
                MTRUnitTestingClusterSimpleStruct * newElement_2;
                newElement_2 = [MTRUnitTestingClusterSimpleStruct new];
                newElement_2.a = [NSNumber numberWithUnsignedChar:entry_2.a];
                newElement_2.b = [NSNumber numberWithBool:entry_2.b];
                newElement_2.c = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_2.c)];
                newElement_2.d = AsData(entry_2.d);
                newElement_2.e = AsString(entry_2.e);
                if (newElement_2.e == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    return err;
                }
                newElement_2.f = [NSNumber numberWithUnsignedChar:entry_2.f.Raw()];
                newElement_2.g = [NSNumber numberWithFloat:entry_2.g];
                newElement_2.h = [NSNumber numberWithDouble:entry_2.h];
                [array_2 addObject:newElement_2];
            }
            CHIP_ERROR err = iter_2.GetStatus();
            if (err != CHIP_NO_ERROR) {
                return err;
            }
            newElement_0.d = array_2;
        }
        { // Scope for our temporary variables
            auto * array_2 = [NSMutableArray new];
            auto iter_2 = entry_0.e.begin();
            while (iter_2.Next()) {
                auto & entry_2 = iter_2.GetValue();
                NSNumber * newElement_2;
                newElement_2 = [NSNumber numberWithUnsignedInt:entry_2];
                [array_2 addObject:newElement_2];
            }
            CHIP_ERROR err = iter_2.GetStatus();
            if (err != CHIP_NO_ERROR) {
                return err;
            }
            newElement_0.e = array_2;
        }
        { // Scope for our temporary variables
            auto * array_2 = [NSMutableArray new];
            auto iter_2 = entry_0.f.begin();
            while (iter_2.Next()) {
                auto & entry_2 = iter_2.GetValue();
                NSData * newElement_2;
                newElement_2 = AsData(entry_2);
                [array_2 addObject:newElement_2];
            }
            CHIP_ERROR err = iter_2.GetStatus();
            if (err != CHIP_NO_ERROR) {
                return err;
            }
            newElement_0.f = array_2;
        }
        { // Scope for our temporary variables
            auto * array_2 = [NSMutableArray new];
            auto iter_2 = entry_0.g.begin();
            while (iter_2.Next()) {
                auto & entry_2 = iter_2.GetValue();
                NSNumber * newElement_2;
                newElement_2 = [NSNumber numberWithUnsignedChar:entry_2];
                [array_2 addObject:newElement_2];
            }
            CHIP_ERROR err = iter_2.GetStatus();
            if (err != CHIP_NO_ERROR) {
                return err;
            }
            newElement_0.g = array_2;
        }
        [array_0 addObject:newElement_0];
    }
    CHIP_ERROR err = iter_0.GetStatus();
    if (err != CHIP_NO_ERROR) {
        return err;
    }
    self.arg1 = array_0;
}
}
{
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = decodableStruct.arg2.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            MTRUnitTestingClusterSimpleStruct * newElement_0;
            newElement_0 = [MTRUnitTestingClusterSimpleStruct new];
            newElement_0.a = [NSNumber numberWithUnsignedChar:entry_0.a];
            newElement_0.b = [NSNumber numberWithBool:entry_0.b];
            newElement_0.c = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.c)];
            newElement_0.d = AsData(entry_0.d);
            newElement_0.e = AsString(entry_0.e);
            if (newElement_0.e == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                return err;
            }
            newElement_0.f = [NSNumber numberWithUnsignedChar:entry_0.f.Raw()];
            newElement_0.g = [NSNumber numberWithFloat:entry_0.g];
            newElement_0.h = [NSNumber numberWithDouble:entry_0.h];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            return err;
        }
        self.arg2 = array_0;
    }
}
{
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = decodableStruct.arg3.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0)];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            return err;
        }
        self.arg3 = array_0;
    }
}
{
    { // Scope for our temporary variables
        auto * array_0 = [NSMutableArray new];
        auto iter_0 = decodableStruct.arg4.begin();
        while (iter_0.Next()) {
            auto & entry_0 = iter_0.GetValue();
            NSNumber * newElement_0;
            newElement_0 = [NSNumber numberWithBool:entry_0];
            [array_0 addObject:newElement_0];
        }
        CHIP_ERROR err = iter_0.GetStatus();
        if (err != CHIP_NO_ERROR) {
            return err;
        }
        self.arg4 = array_0;
    }
}
{
    self.arg5 = [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.arg5)];
}
{
    self.arg6 = [NSNumber numberWithBool:decodableStruct.arg6];
}
return CHIP_NO_ERROR;
}
@end

@implementation MTRTestClusterClusterTestStructArrayArgumentResponseParams
@end
@implementation MTRUnitTestingClusterTestAddArgumentsParams
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = @(0);

        _arg2 = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTestAddArgumentsParams alloc] init];

    other.arg1 = self.arg1;
    other.arg2 = self.arg2;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: arg1:%@; arg2:%@; >", NSStringFromClass([self class]), _arg1, _arg2];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterTestAddArgumentsParams
@end
@implementation MTRUnitTestingClusterTestListInt8UReverseResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = [NSArray array];
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTestListInt8UReverseResponseParams alloc] init];

    other.arg1 = self.arg1;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; >", NSStringFromClass([self class]), _arg1];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::UnitTesting::Commands::TestListInt8UReverseResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRUnitTestingClusterTestListInt8UReverseResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::UnitTesting::Commands::TestListInt8UReverseResponse::DecodableType &)decodableStruct
{
    {
        { // Scope for our temporary variables
            auto * array_0 = [NSMutableArray new];
            auto iter_0 = decodableStruct.arg1.begin();
            while (iter_0.Next()) {
                auto & entry_0 = iter_0.GetValue();
                NSNumber * newElement_0;
                newElement_0 = [NSNumber numberWithUnsignedChar:entry_0];
                [array_0 addObject:newElement_0];
            }
            CHIP_ERROR err = iter_0.GetStatus();
            if (err != CHIP_NO_ERROR) {
                return err;
            }
            self.arg1 = array_0;
        }
    }
    return CHIP_NO_ERROR;
}
@end

@implementation MTRTestClusterClusterTestListInt8UReverseResponseParams
@end
@implementation MTRUnitTestingClusterTestSimpleArgumentRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTestSimpleArgumentRequestParams alloc] init];

    other.arg1 = self.arg1;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; >", NSStringFromClass([self class]), _arg1];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterTestSimpleArgumentRequestParams
@end
@implementation MTRUnitTestingClusterTestEnumsResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = @(0);

        _arg2 = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTestEnumsResponseParams alloc] init];

    other.arg1 = self.arg1;
    other.arg2 = self.arg2;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: arg1:%@; arg2:%@; >", NSStringFromClass([self class]), _arg1, _arg2];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::UnitTesting::Commands::TestEnumsResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRUnitTestingClusterTestEnumsResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::UnitTesting::Commands::TestEnumsResponse::DecodableType &)decodableStruct
{
    {
        self.arg1 = [NSNumber numberWithUnsignedShort:chip::to_underlying(decodableStruct.arg1)];
    }
    {
        self.arg2 = [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.arg2)];
    }
    return CHIP_NO_ERROR;
}
@end

@implementation MTRTestClusterClusterTestEnumsResponseParams
@end
@implementation MTRUnitTestingClusterTestStructArrayArgumentRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = [NSArray array];

        _arg2 = [NSArray array];

        _arg3 = [NSArray array];

        _arg4 = [NSArray array];

        _arg5 = @(0);

        _arg6 = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTestStructArrayArgumentRequestParams alloc] init];

    other.arg1 = self.arg1;
    other.arg2 = self.arg2;
    other.arg3 = self.arg3;
    other.arg4 = self.arg4;
    other.arg5 = self.arg5;
    other.arg6 = self.arg6;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; arg2:%@; arg3:%@; arg4:%@; arg5:%@; arg6:%@; >",
                                             NSStringFromClass([self class]), _arg1, _arg2, _arg3, _arg4, _arg5, _arg6];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterTestStructArrayArgumentRequestParams
@end
@implementation MTRUnitTestingClusterTestNullableOptionalResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _wasPresent = @(0);

        _wasNull = nil;

        _value = nil;

        _originalValue = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTestNullableOptionalResponseParams alloc] init];

    other.wasPresent = self.wasPresent;
    other.wasNull = self.wasNull;
    other.value = self.value;
    other.originalValue = self.originalValue;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: wasPresent:%@; wasNull:%@; value:%@; originalValue:%@; >",
                                             NSStringFromClass([self class]), _wasPresent, _wasNull, _value, _originalValue];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::UnitTesting::Commands::TestNullableOptionalResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRUnitTestingClusterTestNullableOptionalResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::UnitTesting::Commands::TestNullableOptionalResponse::DecodableType &)decodableStruct
{
    {
        self.wasPresent = [NSNumber numberWithBool:decodableStruct.wasPresent];
    }
    {
        if (decodableStruct.wasNull.HasValue()) {
            self.wasNull = [NSNumber numberWithBool:decodableStruct.wasNull.Value()];
        } else {
            self.wasNull = nil;
        }
    }
    {
        if (decodableStruct.value.HasValue()) {
            self.value = [NSNumber numberWithUnsignedChar:decodableStruct.value.Value()];
        } else {
            self.value = nil;
        }
    }
    {
        if (decodableStruct.originalValue.HasValue()) {
            if (decodableStruct.originalValue.Value().IsNull()) {
                self.originalValue = nil;
            } else {
                self.originalValue = [NSNumber numberWithUnsignedChar:decodableStruct.originalValue.Value().Value()];
            }
        } else {
            self.originalValue = nil;
        }
    }
    return CHIP_NO_ERROR;
}
@end

@implementation MTRTestClusterClusterTestNullableOptionalResponseParams
@end
@implementation MTRUnitTestingClusterTestStructArgumentRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = [MTRUnitTestingClusterSimpleStruct new];
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTestStructArgumentRequestParams alloc] init];

    other.arg1 = self.arg1;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; >", NSStringFromClass([self class]), _arg1];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterTestStructArgumentRequestParams
@end
@implementation MTRUnitTestingClusterTestComplexNullableOptionalResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _nullableIntWasNull = @(0);

        _nullableIntValue = nil;

        _optionalIntWasPresent = @(0);

        _optionalIntValue = nil;

        _nullableOptionalIntWasPresent = @(0);

        _nullableOptionalIntWasNull = nil;

        _nullableOptionalIntValue = nil;

        _nullableStringWasNull = @(0);

        _nullableStringValue = nil;

        _optionalStringWasPresent = @(0);

        _optionalStringValue = nil;

        _nullableOptionalStringWasPresent = @(0);

        _nullableOptionalStringWasNull = nil;

        _nullableOptionalStringValue = nil;

        _nullableStructWasNull = @(0);

        _nullableStructValue = nil;

        _optionalStructWasPresent = @(0);

        _optionalStructValue = nil;

        _nullableOptionalStructWasPresent = @(0);

        _nullableOptionalStructWasNull = nil;

        _nullableOptionalStructValue = nil;

        _nullableListWasNull = @(0);

        _nullableListValue = nil;

        _optionalListWasPresent = @(0);

        _optionalListValue = nil;

        _nullableOptionalListWasPresent = @(0);

        _nullableOptionalListWasNull = nil;

        _nullableOptionalListValue = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTestComplexNullableOptionalResponseParams alloc] init];

    other.nullableIntWasNull = self.nullableIntWasNull;
    other.nullableIntValue = self.nullableIntValue;
    other.optionalIntWasPresent = self.optionalIntWasPresent;
    other.optionalIntValue = self.optionalIntValue;
    other.nullableOptionalIntWasPresent = self.nullableOptionalIntWasPresent;
    other.nullableOptionalIntWasNull = self.nullableOptionalIntWasNull;
    other.nullableOptionalIntValue = self.nullableOptionalIntValue;
    other.nullableStringWasNull = self.nullableStringWasNull;
    other.nullableStringValue = self.nullableStringValue;
    other.optionalStringWasPresent = self.optionalStringWasPresent;
    other.optionalStringValue = self.optionalStringValue;
    other.nullableOptionalStringWasPresent = self.nullableOptionalStringWasPresent;
    other.nullableOptionalStringWasNull = self.nullableOptionalStringWasNull;
    other.nullableOptionalStringValue = self.nullableOptionalStringValue;
    other.nullableStructWasNull = self.nullableStructWasNull;
    other.nullableStructValue = self.nullableStructValue;
    other.optionalStructWasPresent = self.optionalStructWasPresent;
    other.optionalStructValue = self.optionalStructValue;
    other.nullableOptionalStructWasPresent = self.nullableOptionalStructWasPresent;
    other.nullableOptionalStructWasNull = self.nullableOptionalStructWasNull;
    other.nullableOptionalStructValue = self.nullableOptionalStructValue;
    other.nullableListWasNull = self.nullableListWasNull;
    other.nullableListValue = self.nullableListValue;
    other.optionalListWasPresent = self.optionalListWasPresent;
    other.optionalListValue = self.optionalListValue;
    other.nullableOptionalListWasPresent = self.nullableOptionalListWasPresent;
    other.nullableOptionalListWasNull = self.nullableOptionalListWasNull;
    other.nullableOptionalListValue = self.nullableOptionalListValue;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: nullableIntWasNull:%@; nullableIntValue:%@; optionalIntWasPresent:%@; optionalIntValue:%@; "
                         @"nullableOptionalIntWasPresent:%@; nullableOptionalIntWasNull:%@; nullableOptionalIntValue:%@; "
                         @"nullableStringWasNull:%@; nullableStringValue:%@; optionalStringWasPresent:%@; optionalStringValue:%@; "
                         @"nullableOptionalStringWasPresent:%@; nullableOptionalStringWasNull:%@; nullableOptionalStringValue:%@; "
                         @"nullableStructWasNull:%@; nullableStructValue:%@; optionalStructWasPresent:%@; optionalStructValue:%@; "
                         @"nullableOptionalStructWasPresent:%@; nullableOptionalStructWasNull:%@; nullableOptionalStructValue:%@; "
                         @"nullableListWasNull:%@; nullableListValue:%@; optionalListWasPresent:%@; optionalListValue:%@; "
                         @"nullableOptionalListWasPresent:%@; nullableOptionalListWasNull:%@; nullableOptionalListValue:%@; >",
        NSStringFromClass([self class]), _nullableIntWasNull, _nullableIntValue, _optionalIntWasPresent, _optionalIntValue,
        _nullableOptionalIntWasPresent, _nullableOptionalIntWasNull, _nullableOptionalIntValue, _nullableStringWasNull,
        _nullableStringValue, _optionalStringWasPresent, _optionalStringValue, _nullableOptionalStringWasPresent,
        _nullableOptionalStringWasNull, _nullableOptionalStringValue, _nullableStructWasNull, _nullableStructValue,
        _optionalStructWasPresent, _optionalStructValue, _nullableOptionalStructWasPresent, _nullableOptionalStructWasNull,
        _nullableOptionalStructValue, _nullableListWasNull, _nullableListValue, _optionalListWasPresent, _optionalListValue,
        _nullableOptionalListWasPresent, _nullableOptionalListWasNull, _nullableOptionalListValue];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::UnitTesting::Commands::TestComplexNullableOptionalResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRUnitTestingClusterTestComplexNullableOptionalResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::UnitTesting::Commands::TestComplexNullableOptionalResponse::DecodableType &)decodableStruct
{
    {
        self.nullableIntWasNull = [NSNumber numberWithBool:decodableStruct.nullableIntWasNull];
    }
    {
        if (decodableStruct.nullableIntValue.HasValue()) {
            self.nullableIntValue = [NSNumber numberWithUnsignedShort:decodableStruct.nullableIntValue.Value()];
        } else {
            self.nullableIntValue = nil;
        }
    }
    {
        self.optionalIntWasPresent = [NSNumber numberWithBool:decodableStruct.optionalIntWasPresent];
    }
    {
        if (decodableStruct.optionalIntValue.HasValue()) {
            self.optionalIntValue = [NSNumber numberWithUnsignedShort:decodableStruct.optionalIntValue.Value()];
        } else {
            self.optionalIntValue = nil;
        }
    }
    {
        self.nullableOptionalIntWasPresent = [NSNumber numberWithBool:decodableStruct.nullableOptionalIntWasPresent];
    }
    {
        if (decodableStruct.nullableOptionalIntWasNull.HasValue()) {
            self.nullableOptionalIntWasNull = [NSNumber numberWithBool:decodableStruct.nullableOptionalIntWasNull.Value()];
        } else {
            self.nullableOptionalIntWasNull = nil;
        }
    }
    {
        if (decodableStruct.nullableOptionalIntValue.HasValue()) {
            self.nullableOptionalIntValue = [NSNumber numberWithUnsignedShort:decodableStruct.nullableOptionalIntValue.Value()];
        } else {
            self.nullableOptionalIntValue = nil;
        }
    }
    {
        self.nullableStringWasNull = [NSNumber numberWithBool:decodableStruct.nullableStringWasNull];
    }
    {
        if (decodableStruct.nullableStringValue.HasValue()) {
            self.nullableStringValue = AsString(decodableStruct.nullableStringValue.Value());
            if (self.nullableStringValue == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                return err;
            }
        } else {
            self.nullableStringValue = nil;
        }
    }
    {
        self.optionalStringWasPresent = [NSNumber numberWithBool:decodableStruct.optionalStringWasPresent];
    }
    {
        if (decodableStruct.optionalStringValue.HasValue()) {
            self.optionalStringValue = AsString(decodableStruct.optionalStringValue.Value());
            if (self.optionalStringValue == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                return err;
            }
        } else {
            self.optionalStringValue = nil;
        }
    }
    {
        self.nullableOptionalStringWasPresent = [NSNumber numberWithBool:decodableStruct.nullableOptionalStringWasPresent];
    }
    {
        if (decodableStruct.nullableOptionalStringWasNull.HasValue()) {
            self.nullableOptionalStringWasNull = [NSNumber numberWithBool:decodableStruct.nullableOptionalStringWasNull.Value()];
        } else {
            self.nullableOptionalStringWasNull = nil;
        }
    }
    {
        if (decodableStruct.nullableOptionalStringValue.HasValue()) {
            self.nullableOptionalStringValue = AsString(decodableStruct.nullableOptionalStringValue.Value());
            if (self.nullableOptionalStringValue == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                return err;
            }
        } else {
            self.nullableOptionalStringValue = nil;
        }
    }
    {
        self.nullableStructWasNull = [NSNumber numberWithBool:decodableStruct.nullableStructWasNull];
    }
    {
        if (decodableStruct.nullableStructValue.HasValue()) {
            self.nullableStructValue = [MTRUnitTestingClusterSimpleStruct new];
            self.nullableStructValue.a = [NSNumber numberWithUnsignedChar:decodableStruct.nullableStructValue.Value().a];
            self.nullableStructValue.b = [NSNumber numberWithBool:decodableStruct.nullableStructValue.Value().b];
            self.nullableStructValue.c =
                [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.nullableStructValue.Value().c)];
            self.nullableStructValue.d = AsData(decodableStruct.nullableStructValue.Value().d);
            self.nullableStructValue.e = AsString(decodableStruct.nullableStructValue.Value().e);
            if (self.nullableStructValue.e == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                return err;
            }
            self.nullableStructValue.f = [NSNumber numberWithUnsignedChar:decodableStruct.nullableStructValue.Value().f.Raw()];
            self.nullableStructValue.g = [NSNumber numberWithFloat:decodableStruct.nullableStructValue.Value().g];
            self.nullableStructValue.h = [NSNumber numberWithDouble:decodableStruct.nullableStructValue.Value().h];
        } else {
            self.nullableStructValue = nil;
        }
    }
    {
        self.optionalStructWasPresent = [NSNumber numberWithBool:decodableStruct.optionalStructWasPresent];
    }
    {
        if (decodableStruct.optionalStructValue.HasValue()) {
            self.optionalStructValue = [MTRUnitTestingClusterSimpleStruct new];
            self.optionalStructValue.a = [NSNumber numberWithUnsignedChar:decodableStruct.optionalStructValue.Value().a];
            self.optionalStructValue.b = [NSNumber numberWithBool:decodableStruct.optionalStructValue.Value().b];
            self.optionalStructValue.c =
                [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.optionalStructValue.Value().c)];
            self.optionalStructValue.d = AsData(decodableStruct.optionalStructValue.Value().d);
            self.optionalStructValue.e = AsString(decodableStruct.optionalStructValue.Value().e);
            if (self.optionalStructValue.e == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                return err;
            }
            self.optionalStructValue.f = [NSNumber numberWithUnsignedChar:decodableStruct.optionalStructValue.Value().f.Raw()];
            self.optionalStructValue.g = [NSNumber numberWithFloat:decodableStruct.optionalStructValue.Value().g];
            self.optionalStructValue.h = [NSNumber numberWithDouble:decodableStruct.optionalStructValue.Value().h];
        } else {
            self.optionalStructValue = nil;
        }
    }
    {
        self.nullableOptionalStructWasPresent = [NSNumber numberWithBool:decodableStruct.nullableOptionalStructWasPresent];
    }
    {
        if (decodableStruct.nullableOptionalStructWasNull.HasValue()) {
            self.nullableOptionalStructWasNull = [NSNumber numberWithBool:decodableStruct.nullableOptionalStructWasNull.Value()];
        } else {
            self.nullableOptionalStructWasNull = nil;
        }
    }
    {
        if (decodableStruct.nullableOptionalStructValue.HasValue()) {
            self.nullableOptionalStructValue = [MTRUnitTestingClusterSimpleStruct new];
            self.nullableOptionalStructValue.a =
                [NSNumber numberWithUnsignedChar:decodableStruct.nullableOptionalStructValue.Value().a];
            self.nullableOptionalStructValue.b = [NSNumber numberWithBool:decodableStruct.nullableOptionalStructValue.Value().b];
            self.nullableOptionalStructValue.c =
                [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.nullableOptionalStructValue.Value().c)];
            self.nullableOptionalStructValue.d = AsData(decodableStruct.nullableOptionalStructValue.Value().d);
            self.nullableOptionalStructValue.e = AsString(decodableStruct.nullableOptionalStructValue.Value().e);
            if (self.nullableOptionalStructValue.e == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                return err;
            }
            self.nullableOptionalStructValue.f =
                [NSNumber numberWithUnsignedChar:decodableStruct.nullableOptionalStructValue.Value().f.Raw()];
            self.nullableOptionalStructValue.g = [NSNumber numberWithFloat:decodableStruct.nullableOptionalStructValue.Value().g];
            self.nullableOptionalStructValue.h = [NSNumber numberWithDouble:decodableStruct.nullableOptionalStructValue.Value().h];
        } else {
            self.nullableOptionalStructValue = nil;
        }
    }
    {
        self.nullableListWasNull = [NSNumber numberWithBool:decodableStruct.nullableListWasNull];
    }
    {
        if (decodableStruct.nullableListValue.HasValue()) {
            { // Scope for our temporary variables
                auto * array_1 = [NSMutableArray new];
                auto iter_1 = decodableStruct.nullableListValue.Value().begin();
                while (iter_1.Next()) {
                    auto & entry_1 = iter_1.GetValue();
                    NSNumber * newElement_1;
                    newElement_1 = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_1)];
                    [array_1 addObject:newElement_1];
                }
                CHIP_ERROR err = iter_1.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    return err;
                }
                self.nullableListValue = array_1;
            }
        } else {
            self.nullableListValue = nil;
        }
    }
    {
        self.optionalListWasPresent = [NSNumber numberWithBool:decodableStruct.optionalListWasPresent];
    }
    {
        if (decodableStruct.optionalListValue.HasValue()) {
            { // Scope for our temporary variables
                auto * array_1 = [NSMutableArray new];
                auto iter_1 = decodableStruct.optionalListValue.Value().begin();
                while (iter_1.Next()) {
                    auto & entry_1 = iter_1.GetValue();
                    NSNumber * newElement_1;
                    newElement_1 = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_1)];
                    [array_1 addObject:newElement_1];
                }
                CHIP_ERROR err = iter_1.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    return err;
                }
                self.optionalListValue = array_1;
            }
        } else {
            self.optionalListValue = nil;
        }
    }
    {
        self.nullableOptionalListWasPresent = [NSNumber numberWithBool:decodableStruct.nullableOptionalListWasPresent];
    }
    {
        if (decodableStruct.nullableOptionalListWasNull.HasValue()) {
            self.nullableOptionalListWasNull = [NSNumber numberWithBool:decodableStruct.nullableOptionalListWasNull.Value()];
        } else {
            self.nullableOptionalListWasNull = nil;
        }
    }
    {
        if (decodableStruct.nullableOptionalListValue.HasValue()) {
            { // Scope for our temporary variables
                auto * array_1 = [NSMutableArray new];
                auto iter_1 = decodableStruct.nullableOptionalListValue.Value().begin();
                while (iter_1.Next()) {
                    auto & entry_1 = iter_1.GetValue();
                    NSNumber * newElement_1;
                    newElement_1 = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_1)];
                    [array_1 addObject:newElement_1];
                }
                CHIP_ERROR err = iter_1.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    return err;
                }
                self.nullableOptionalListValue = array_1;
            }
        } else {
            self.nullableOptionalListValue = nil;
        }
    }
    return CHIP_NO_ERROR;
}
@end

@implementation MTRTestClusterClusterTestComplexNullableOptionalResponseParams
@end
@implementation MTRUnitTestingClusterTestNestedStructArgumentRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = [MTRUnitTestingClusterNestedStruct new];
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTestNestedStructArgumentRequestParams alloc] init];

    other.arg1 = self.arg1;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; >", NSStringFromClass([self class]), _arg1];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterTestNestedStructArgumentRequestParams
@end
@implementation MTRUnitTestingClusterBooleanResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _value = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterBooleanResponseParams alloc] init];

    other.value = self.value;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: value:%@; >", NSStringFromClass([self class]), _value];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::UnitTesting::Commands::BooleanResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRUnitTestingClusterBooleanResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::UnitTesting::Commands::BooleanResponse::DecodableType &)decodableStruct
{
    {
        self.value = [NSNumber numberWithBool:decodableStruct.value];
    }
    return CHIP_NO_ERROR;
}
@end

@implementation MTRTestClusterClusterBooleanResponseParams
@end
@implementation MTRUnitTestingClusterTestListStructArgumentRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = [NSArray array];
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTestListStructArgumentRequestParams alloc] init];

    other.arg1 = self.arg1;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; >", NSStringFromClass([self class]), _arg1];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterTestListStructArgumentRequestParams
@end
@implementation MTRUnitTestingClusterSimpleStructResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = [MTRUnitTestingClusterSimpleStruct new];
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterSimpleStructResponseParams alloc] init];

    other.arg1 = self.arg1;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; >", NSStringFromClass([self class]), _arg1];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::UnitTesting::Commands::SimpleStructResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRUnitTestingClusterSimpleStructResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::UnitTesting::Commands::SimpleStructResponse::DecodableType &)decodableStruct
{
    {
        self.arg1 = [MTRUnitTestingClusterSimpleStruct new];
        self.arg1.a = [NSNumber numberWithUnsignedChar:decodableStruct.arg1.a];
        self.arg1.b = [NSNumber numberWithBool:decodableStruct.arg1.b];
        self.arg1.c = [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.arg1.c)];
        self.arg1.d = AsData(decodableStruct.arg1.d);
        self.arg1.e = AsString(decodableStruct.arg1.e);
        if (self.arg1.e == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            return err;
        }
        self.arg1.f = [NSNumber numberWithUnsignedChar:decodableStruct.arg1.f.Raw()];
        self.arg1.g = [NSNumber numberWithFloat:decodableStruct.arg1.g];
        self.arg1.h = [NSNumber numberWithDouble:decodableStruct.arg1.h];
    }
    return CHIP_NO_ERROR;
}
@end

@implementation MTRTestClusterClusterSimpleStructResponseParams
@end
@implementation MTRUnitTestingClusterTestListInt8UArgumentRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = [NSArray array];
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTestListInt8UArgumentRequestParams alloc] init];

    other.arg1 = self.arg1;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; >", NSStringFromClass([self class]), _arg1];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterTestListInt8UArgumentRequestParams
@end
@implementation MTRUnitTestingClusterTestEmitTestEventResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _value = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTestEmitTestEventResponseParams alloc] init];

    other.value = self.value;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: value:%@; >", NSStringFromClass([self class]), _value];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::UnitTesting::Commands::TestEmitTestEventResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRUnitTestingClusterTestEmitTestEventResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::UnitTesting::Commands::TestEmitTestEventResponse::DecodableType &)decodableStruct
{
    {
        self.value = [NSNumber numberWithUnsignedLongLong:decodableStruct.value];
    }
    return CHIP_NO_ERROR;
}
@end

@implementation MTRTestClusterClusterTestEmitTestEventResponseParams
@end
@implementation MTRUnitTestingClusterTestNestedStructListArgumentRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = [MTRUnitTestingClusterNestedStructList new];
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTestNestedStructListArgumentRequestParams alloc] init];

    other.arg1 = self.arg1;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; >", NSStringFromClass([self class]), _arg1];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterTestNestedStructListArgumentRequestParams
@end
@implementation MTRUnitTestingClusterTestEmitTestFabricScopedEventResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _value = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTestEmitTestFabricScopedEventResponseParams alloc] init];

    other.value = self.value;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: value:%@; >", NSStringFromClass([self class]), _value];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    using DecodableType = chip::app::Clusters::UnitTesting::Commands::TestEmitTestFabricScopedEventResponse::DecodableType;
    chip::System::PacketBufferHandle buffer = [MTRBaseDevice _responseDataForCommand:responseValue
                                                                           clusterID:DecodableType::GetClusterId()
                                                                           commandID:DecodableType::GetCommandId()
                                                                               error:error];
    if (buffer.IsNull()) {
        return nil;
    }

    chip::TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    CHIP_ERROR err = reader.Next(chip::TLV::AnonymousTag());
    if (err == CHIP_NO_ERROR) {
        DecodableType decodedStruct;
        err = chip::app::DataModel::Decode(reader, decodedStruct);
        if (err == CHIP_NO_ERROR) {
            err = [self _setFieldsFromDecodableStruct:decodedStruct];
            if (err == CHIP_NO_ERROR) {
                return self;
            }
        }
    }

    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @{ NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
    return nil;
}

@end

@implementation MTRUnitTestingClusterTestEmitTestFabricScopedEventResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::UnitTesting::Commands::TestEmitTestFabricScopedEventResponse::DecodableType &)decodableStruct
{
    {
        self.value = [NSNumber numberWithUnsignedLongLong:decodableStruct.value];
    }
    return CHIP_NO_ERROR;
}
@end

@implementation MTRTestClusterClusterTestEmitTestFabricScopedEventResponseParams
@end
@implementation MTRUnitTestingClusterTestListNestedStructListArgumentRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = [NSArray array];
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTestListNestedStructListArgumentRequestParams alloc] init];

    other.arg1 = self.arg1;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; >", NSStringFromClass([self class]), _arg1];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterTestListNestedStructListArgumentRequestParams
@end
@implementation MTRUnitTestingClusterTestListInt8UReverseRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = [NSArray array];
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTestListInt8UReverseRequestParams alloc] init];

    other.arg1 = self.arg1;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; >", NSStringFromClass([self class]), _arg1];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterTestListInt8UReverseRequestParams
@end
@implementation MTRUnitTestingClusterTestEnumsRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = @(0);

        _arg2 = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTestEnumsRequestParams alloc] init];

    other.arg1 = self.arg1;
    other.arg2 = self.arg2;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: arg1:%@; arg2:%@; >", NSStringFromClass([self class]), _arg1, _arg2];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterTestEnumsRequestParams
@end
@implementation MTRUnitTestingClusterTestNullableOptionalRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTestNullableOptionalRequestParams alloc] init];

    other.arg1 = self.arg1;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; >", NSStringFromClass([self class]), _arg1];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterTestNullableOptionalRequestParams
@end
@implementation MTRUnitTestingClusterTestComplexNullableOptionalRequestParams
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
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTestComplexNullableOptionalRequestParams alloc] init];

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
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

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

@implementation MTRTestClusterClusterTestComplexNullableOptionalRequestParams
@end
@implementation MTRUnitTestingClusterSimpleStructEchoRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = [MTRUnitTestingClusterSimpleStruct new];
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterSimpleStructEchoRequestParams alloc] init];

    other.arg1 = self.arg1;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; >", NSStringFromClass([self class]), _arg1];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterSimpleStructEchoRequestParams
@end
@implementation MTRUnitTestingClusterTimedInvokeRequestParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTimedInvokeRequestParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterTimedInvokeRequestParams
@end
@implementation MTRUnitTestingClusterTestSimpleOptionalArgumentRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = nil;
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTestSimpleOptionalArgumentRequestParams alloc] init];

    other.arg1 = self.arg1;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; >", NSStringFromClass([self class]), _arg1];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterTestSimpleOptionalArgumentRequestParams
@end
@implementation MTRUnitTestingClusterTestEmitTestEventRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = @(0);

        _arg2 = @(0);

        _arg3 = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTestEmitTestEventRequestParams alloc] init];

    other.arg1 = self.arg1;
    other.arg2 = self.arg2;
    other.arg3 = self.arg3;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: arg1:%@; arg2:%@; arg3:%@; >", NSStringFromClass([self class]), _arg1, _arg2, _arg3];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterTestEmitTestEventRequestParams
@end
@implementation MTRUnitTestingClusterTestEmitTestFabricScopedEventRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRUnitTestingClusterTestEmitTestFabricScopedEventRequestParams alloc] init];

    other.arg1 = self.arg1;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; >", NSStringFromClass([self class]), _arg1];
    return descriptionString;
}

@end

@implementation MTRTestClusterClusterTestEmitTestFabricScopedEventRequestParams
@end
@implementation MTRFaultInjectionClusterFailAtFaultParams
- (instancetype)init
{
    if (self = [super init]) {

        _type = @(0);

        _id = @(0);

        _numCallsToSkip = @(0);

        _numCallsToFail = @(0);

        _takeMutex = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRFaultInjectionClusterFailAtFaultParams alloc] init];

    other.type = self.type;
    other.id = self.id;
    other.numCallsToSkip = self.numCallsToSkip;
    other.numCallsToFail = self.numCallsToFail;
    other.takeMutex = self.takeMutex;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: type:%@; id:%@; numCallsToSkip:%@; numCallsToFail:%@; takeMutex:%@; >",
                  NSStringFromClass([self class]), _type, _id, _numCallsToSkip, _numCallsToFail, _takeMutex];
    return descriptionString;
}

@end
@implementation MTRFaultInjectionClusterFailRandomlyAtFaultParams
- (instancetype)init
{
    if (self = [super init]) {

        _type = @(0);

        _id = @(0);

        _percentage = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRFaultInjectionClusterFailRandomlyAtFaultParams alloc] init];

    other.type = self.type;
    other.id = self.id;
    other.percentage = self.percentage;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: type:%@; id:%@; percentage:%@; >", NSStringFromClass([self class]), _type, _id, _percentage];
    return descriptionString;
}

@end

NS_ASSUME_NONNULL_END
