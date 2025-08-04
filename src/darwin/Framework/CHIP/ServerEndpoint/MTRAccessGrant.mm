/**
 *    Copyright (c) 2024 Project CHIP Authors
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

#import "MTRDefines_Internal.h"
#import "MTRLogging_Internal.h"
#import <Matter/MTRAccessGrant.h>

#include <lib/core/CASEAuthTag.h>
#include <lib/core/GroupId.h>
#include <lib/core/NodeId.h>
#include <lib/support/SafeInt.h>

using namespace chip;

MTR_DIRECT_MEMBERS
@implementation MTRAccessGrant

+ (nullable MTRAccessGrant *)accessGrantForNodeID:(NSNumber *)nodeID privilege:(MTRAccessControlEntryPrivilege)privilege
{
    NodeId id = nodeID.unsignedLongLongValue;
    if (!IsOperationalNodeId(id)) {
        MTR_LOG_ERROR("MTRAccessGrant provided non-operational node ID: 0x%llx", id);
        return nil;
    }

    return [[MTRAccessGrant alloc] initWithSubject:[nodeID copy] privilege:privilege authenticationMode:MTRAccessControlEntryAuthModeCASE];
}

+ (nullable MTRAccessGrant *)accessGrantForCASEAuthenticatedTag:(NSNumber *)caseAuthenticatedTag privilege:(MTRAccessControlEntryPrivilege)privilege
{
    auto value = caseAuthenticatedTag.unsignedLongLongValue;
    if (!CanCastTo<CASEAuthTag>(value)) {
        MTR_LOG_ERROR("MTRAccessGrant provided too-large CAT value: 0x%llx", value);
        return nil;
    }

    CASEAuthTag tag = static_cast<CASEAuthTag>(value);
    if (!IsValidCASEAuthTag(tag)) {
        MTR_LOG_ERROR("MTRAccessGrant provided invalid CAT value: 0x%" PRIx32, tag);
        return nil;
    }

    return [[MTRAccessGrant alloc] initWithSubject:@(NodeIdFromCASEAuthTag(tag)) privilege:privilege authenticationMode:MTRAccessControlEntryAuthModeCASE];
}

+ (nullable MTRAccessGrant *)accessGrantForGroupID:(NSNumber *)groupID privilege:(MTRAccessControlEntryPrivilege)privilege
{
    auto value = groupID.unsignedLongLongValue;
    if (!CanCastTo<GroupId>(value)) {
        MTR_LOG_ERROR("MTRAccessGrant provided too-large group id: 0x%llx", value);
        return nil;
    }

    GroupId id = static_cast<GroupId>(value);
    if (!IsValidGroupId(id)) {
        MTR_LOG_ERROR("MTRAccessGrant provided invalid group id: 0x%" PRIx32, id);
        return nil;
    }

    return [[MTRAccessGrant alloc] initWithSubject:@(NodeIdFromGroupId(id)) privilege:privilege authenticationMode:MTRAccessControlEntryAuthModeGroup];
}

+ (MTRAccessGrant *)accessGrantForAllNodesWithPrivilege:(MTRAccessControlEntryPrivilege)privilege
{
    return [[MTRAccessGrant alloc] initWithSubject:nil privilege:privilege authenticationMode:MTRAccessControlEntryAuthModeCASE];
}

// initWithSubject assumes that the subject has already been validated and, if
// needed, copied from the input.
- (nullable instancetype)initWithSubject:(nullable NSNumber *)subject privilege:(MTRAccessControlEntryPrivilege)privilege authenticationMode:(MTRAccessControlEntryAuthMode)authenticationMode
{
    if (!(self = [super init])) {
        return nil;
    }

    _subjectID = subject;
    _grantedPrivilege = privilege;
    _authenticationMode = authenticationMode;
    return self;
}

- (id)copyWithZone:(NSZone *)zone
{
    // We have no mutable state.
    return self;
}

- (BOOL)isEqual:(id)object
{
    if ([object class] != [self class]) {
        return NO;
    }

    MTRAccessGrant * other = object;

    BOOL sameSubjectID = (_subjectID == nil && other.subjectID == nil) || [_subjectID isEqual:other.subjectID];
    return sameSubjectID && _grantedPrivilege == other.grantedPrivilege && _authenticationMode == other.authenticationMode;
}

- (NSUInteger)hash
{
    return _subjectID.unsignedIntegerValue ^ _grantedPrivilege ^ _authenticationMode;
}

- (NSString *)description
{
    NSString * privilege = @"Unknown";
    switch (_grantedPrivilege) {
    case MTRAccessControlEntryPrivilegeView:
        privilege = @"View";
        break;
    case MTRAccessControlEntryPrivilegeProxyView:
        privilege = @"ProxyView";
        break;
    case MTRAccessControlEntryPrivilegeOperate:
        privilege = @"Operate";
        break;
    case MTRAccessControlEntryPrivilegeManage:
        privilege = @"Manage";
        break;
    case MTRAccessControlEntryPrivilegeAdminister:
        privilege = @"Administer";
        break;
    }

    if (_subjectID == nil) {
        return [NSString stringWithFormat:@"<%@ all nodes can %@>", self.class, privilege];
    }

    NodeId nodeId = static_cast<NodeId>(_subjectID.unsignedLongLongValue);
    if (IsGroupId(nodeId)) {
        return [NSString stringWithFormat:@"<%@ group 0x%x can %@>", self.class, GroupIdFromNodeId(nodeId), privilege];
    }

    if (IsCASEAuthTag(nodeId)) {
        return [NSString stringWithFormat:@"<%@ nodes with CASE Authenticated Tag 0x%08x can %@>", self.class, CASEAuthTagFromNodeId(nodeId), privilege];
    }

    return [NSString stringWithFormat:@"<%@ node 0x%016llx can %@>", self.class, nodeId, privilege];
}

@end
