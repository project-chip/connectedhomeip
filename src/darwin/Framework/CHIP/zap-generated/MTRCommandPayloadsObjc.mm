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

NS_ASSUME_NONNULL_BEGIN

@implementation MTRIdentifyClusterIdentifyParams
- (instancetype)init
{
    if (self = [super init]) {

        _identifyTime = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRIdentifyClusterIdentifyParams alloc] init];

    other.identifyTime = self.identifyTime;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRIdentifyClusterTriggerEffectParams alloc] init];

    other.effectIdentifier = self.effectIdentifier;
    other.effectVariant = self.effectVariant;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

        _groupId = @(0);

        _groupName = @"";
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGroupsClusterAddGroupParams alloc] init];

    other.groupId = self.groupId;
    other.groupName = self.groupName;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: groupId:%@; groupName:%@; >", NSStringFromClass([self class]), _groupId, _groupName];
    return descriptionString;
}

@end
@implementation MTRGroupsClusterAddGroupResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _groupId = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGroupsClusterAddGroupResponseParams alloc] init];

    other.status = self.status;
    other.groupId = self.groupId;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: status:%@; groupId:%@; >", NSStringFromClass([self class]), _status, _groupId];
    return descriptionString;
}

@end
@implementation MTRGroupsClusterViewGroupParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupId = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGroupsClusterViewGroupParams alloc] init];

    other.groupId = self.groupId;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: groupId:%@; >", NSStringFromClass([self class]), _groupId];
    return descriptionString;
}

@end
@implementation MTRGroupsClusterViewGroupResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _groupId = @(0);

        _groupName = @"";
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGroupsClusterViewGroupResponseParams alloc] init];

    other.status = self.status;
    other.groupId = self.groupId;
    other.groupName = self.groupName;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: status:%@; groupId:%@; groupName:%@; >",
                                             NSStringFromClass([self class]), _status, _groupId, _groupName];
    return descriptionString;
}

@end
@implementation MTRGroupsClusterGetGroupMembershipParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupList = [NSArray array];
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGroupsClusterGetGroupMembershipParams alloc] init];

    other.groupList = self.groupList;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

@end
@implementation MTRGroupsClusterRemoveGroupParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupId = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGroupsClusterRemoveGroupParams alloc] init];

    other.groupId = self.groupId;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: groupId:%@; >", NSStringFromClass([self class]), _groupId];
    return descriptionString;
}

@end
@implementation MTRGroupsClusterRemoveGroupResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _groupId = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGroupsClusterRemoveGroupResponseParams alloc] init];

    other.status = self.status;
    other.groupId = self.groupId;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: status:%@; groupId:%@; >", NSStringFromClass([self class]), _status, _groupId];
    return descriptionString;
}

@end
@implementation MTRGroupsClusterRemoveAllGroupsParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGroupsClusterRemoveAllGroupsParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

        _groupId = @(0);

        _groupName = @"";
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGroupsClusterAddGroupIfIdentifyingParams alloc] init];

    other.groupId = self.groupId;
    other.groupName = self.groupName;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: groupId:%@; groupName:%@; >", NSStringFromClass([self class]), _groupId, _groupName];
    return descriptionString;
}

@end
@implementation MTRScenesClusterAddSceneParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupId = @(0);

        _sceneId = @(0);

        _transitionTime = @(0);

        _sceneName = @"";

        _extensionFieldSets = [NSArray array];
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterAddSceneParams alloc] init];

    other.groupId = self.groupId;
    other.sceneId = self.sceneId;
    other.transitionTime = self.transitionTime;
    other.sceneName = self.sceneName;
    other.extensionFieldSets = self.extensionFieldSets;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: groupId:%@; sceneId:%@; transitionTime:%@; sceneName:%@; extensionFieldSets:%@; >",
                  NSStringFromClass([self class]), _groupId, _sceneId, _transitionTime, _sceneName, _extensionFieldSets];
    return descriptionString;
}

@end
@implementation MTRScenesClusterAddSceneResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _groupId = @(0);

        _sceneId = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterAddSceneResponseParams alloc] init];

    other.status = self.status;
    other.groupId = self.groupId;
    other.sceneId = self.sceneId;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: status:%@; groupId:%@; sceneId:%@; >",
                                             NSStringFromClass([self class]), _status, _groupId, _sceneId];
    return descriptionString;
}

@end
@implementation MTRScenesClusterViewSceneParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupId = @(0);

        _sceneId = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterViewSceneParams alloc] init];

    other.groupId = self.groupId;
    other.sceneId = self.sceneId;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: groupId:%@; sceneId:%@; >", NSStringFromClass([self class]), _groupId, _sceneId];
    return descriptionString;
}

@end
@implementation MTRScenesClusterViewSceneResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _groupId = @(0);

        _sceneId = @(0);

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
    other.groupId = self.groupId;
    other.sceneId = self.sceneId;
    other.transitionTime = self.transitionTime;
    other.sceneName = self.sceneName;
    other.extensionFieldSets = self.extensionFieldSets;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: status:%@; groupId:%@; sceneId:%@; transitionTime:%@; sceneName:%@; extensionFieldSets:%@; >",
        NSStringFromClass([self class]), _status, _groupId, _sceneId, _transitionTime, _sceneName, _extensionFieldSets];
    return descriptionString;
}

@end
@implementation MTRScenesClusterRemoveSceneParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupId = @(0);

        _sceneId = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterRemoveSceneParams alloc] init];

    other.groupId = self.groupId;
    other.sceneId = self.sceneId;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: groupId:%@; sceneId:%@; >", NSStringFromClass([self class]), _groupId, _sceneId];
    return descriptionString;
}

@end
@implementation MTRScenesClusterRemoveSceneResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _groupId = @(0);

        _sceneId = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterRemoveSceneResponseParams alloc] init];

    other.status = self.status;
    other.groupId = self.groupId;
    other.sceneId = self.sceneId;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: status:%@; groupId:%@; sceneId:%@; >",
                                             NSStringFromClass([self class]), _status, _groupId, _sceneId];
    return descriptionString;
}

@end
@implementation MTRScenesClusterRemoveAllScenesParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupId = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterRemoveAllScenesParams alloc] init];

    other.groupId = self.groupId;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: groupId:%@; >", NSStringFromClass([self class]), _groupId];
    return descriptionString;
}

@end
@implementation MTRScenesClusterRemoveAllScenesResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _groupId = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterRemoveAllScenesResponseParams alloc] init];

    other.status = self.status;
    other.groupId = self.groupId;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: status:%@; groupId:%@; >", NSStringFromClass([self class]), _status, _groupId];
    return descriptionString;
}

@end
@implementation MTRScenesClusterStoreSceneParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupId = @(0);

        _sceneId = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterStoreSceneParams alloc] init];

    other.groupId = self.groupId;
    other.sceneId = self.sceneId;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: groupId:%@; sceneId:%@; >", NSStringFromClass([self class]), _groupId, _sceneId];
    return descriptionString;
}

@end
@implementation MTRScenesClusterStoreSceneResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _groupId = @(0);

        _sceneId = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterStoreSceneResponseParams alloc] init];

    other.status = self.status;
    other.groupId = self.groupId;
    other.sceneId = self.sceneId;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: status:%@; groupId:%@; sceneId:%@; >",
                                             NSStringFromClass([self class]), _status, _groupId, _sceneId];
    return descriptionString;
}

@end
@implementation MTRScenesClusterRecallSceneParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupId = @(0);

        _sceneId = @(0);

        _transitionTime = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterRecallSceneParams alloc] init];

    other.groupId = self.groupId;
    other.sceneId = self.sceneId;
    other.transitionTime = self.transitionTime;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: groupId:%@; sceneId:%@; transitionTime:%@; >",
                                             NSStringFromClass([self class]), _groupId, _sceneId, _transitionTime];
    return descriptionString;
}

@end
@implementation MTRScenesClusterGetSceneMembershipParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupId = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterGetSceneMembershipParams alloc] init];

    other.groupId = self.groupId;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: groupId:%@; >", NSStringFromClass([self class]), _groupId];
    return descriptionString;
}

@end
@implementation MTRScenesClusterGetSceneMembershipResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _capacity = nil;

        _groupId = @(0);

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
    other.groupId = self.groupId;
    other.sceneList = self.sceneList;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: status:%@; capacity:%@; groupId:%@; sceneList:%@; >",
                                             NSStringFromClass([self class]), _status, _capacity, _groupId, _sceneList];
    return descriptionString;
}

@end
@implementation MTRScenesClusterEnhancedAddSceneParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupId = @(0);

        _sceneId = @(0);

        _transitionTime = @(0);

        _sceneName = @"";

        _extensionFieldSets = [NSArray array];
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterEnhancedAddSceneParams alloc] init];

    other.groupId = self.groupId;
    other.sceneId = self.sceneId;
    other.transitionTime = self.transitionTime;
    other.sceneName = self.sceneName;
    other.extensionFieldSets = self.extensionFieldSets;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: groupId:%@; sceneId:%@; transitionTime:%@; sceneName:%@; extensionFieldSets:%@; >",
                  NSStringFromClass([self class]), _groupId, _sceneId, _transitionTime, _sceneName, _extensionFieldSets];
    return descriptionString;
}

@end
@implementation MTRScenesClusterEnhancedAddSceneResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _groupId = @(0);

        _sceneId = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterEnhancedAddSceneResponseParams alloc] init];

    other.status = self.status;
    other.groupId = self.groupId;
    other.sceneId = self.sceneId;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: status:%@; groupId:%@; sceneId:%@; >",
                                             NSStringFromClass([self class]), _status, _groupId, _sceneId];
    return descriptionString;
}

@end
@implementation MTRScenesClusterEnhancedViewSceneParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupId = @(0);

        _sceneId = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterEnhancedViewSceneParams alloc] init];

    other.groupId = self.groupId;
    other.sceneId = self.sceneId;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: groupId:%@; sceneId:%@; >", NSStringFromClass([self class]), _groupId, _sceneId];
    return descriptionString;
}

@end
@implementation MTRScenesClusterEnhancedViewSceneResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _groupId = @(0);

        _sceneId = @(0);

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
    other.groupId = self.groupId;
    other.sceneId = self.sceneId;
    other.transitionTime = self.transitionTime;
    other.sceneName = self.sceneName;
    other.extensionFieldSets = self.extensionFieldSets;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: status:%@; groupId:%@; sceneId:%@; transitionTime:%@; sceneName:%@; extensionFieldSets:%@; >",
        NSStringFromClass([self class]), _status, _groupId, _sceneId, _transitionTime, _sceneName, _extensionFieldSets];
    return descriptionString;
}

@end
@implementation MTRScenesClusterCopySceneParams
- (instancetype)init
{
    if (self = [super init]) {

        _mode = @(0);

        _groupIdFrom = @(0);

        _sceneIdFrom = @(0);

        _groupIdTo = @(0);

        _sceneIdTo = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterCopySceneParams alloc] init];

    other.mode = self.mode;
    other.groupIdFrom = self.groupIdFrom;
    other.sceneIdFrom = self.sceneIdFrom;
    other.groupIdTo = self.groupIdTo;
    other.sceneIdTo = self.sceneIdTo;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: mode:%@; groupIdFrom:%@; sceneIdFrom:%@; groupIdTo:%@; sceneIdTo:%@; >",
                  NSStringFromClass([self class]), _mode, _groupIdFrom, _sceneIdFrom, _groupIdTo, _sceneIdTo];
    return descriptionString;
}

@end
@implementation MTRScenesClusterCopySceneResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _groupIdFrom = @(0);

        _sceneIdFrom = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRScenesClusterCopySceneResponseParams alloc] init];

    other.status = self.status;
    other.groupIdFrom = self.groupIdFrom;
    other.sceneIdFrom = self.sceneIdFrom;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: status:%@; groupIdFrom:%@; sceneIdFrom:%@; >",
                                             NSStringFromClass([self class]), _status, _groupIdFrom, _sceneIdFrom];
    return descriptionString;
}

@end
@implementation MTROnOffClusterOffParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROnOffClusterOffParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROnOffClusterOnParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROnOffClusterToggleParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

        _effectId = @(0);

        _effectVariant = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROnOffClusterOffWithEffectParams alloc] init];

    other.effectId = self.effectId;
    other.effectVariant = self.effectVariant;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: effectId:%@; effectVariant:%@; >", NSStringFromClass([self class]), _effectId, _effectVariant];
    return descriptionString;
}

@end
@implementation MTROnOffClusterOnWithRecallGlobalSceneParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROnOffClusterOnWithRecallGlobalSceneParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRLevelControlClusterStopParams alloc] init];

    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRLevelControlClusterStopWithOnOffParams alloc] init];

    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRLevelControlClusterMoveToClosestFrequencyParams alloc] init];

    other.frequency = self.frequency;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRActionsClusterInstantActionParams alloc] init];

    other.actionID = self.actionID;
    other.invokeID = self.invokeID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRActionsClusterStartActionParams alloc] init];

    other.actionID = self.actionID;
    other.invokeID = self.invokeID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRActionsClusterStopActionParams alloc] init];

    other.actionID = self.actionID;
    other.invokeID = self.invokeID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRActionsClusterPauseActionParams alloc] init];

    other.actionID = self.actionID;
    other.invokeID = self.invokeID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRActionsClusterResumeActionParams alloc] init];

    other.actionID = self.actionID;
    other.invokeID = self.invokeID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRActionsClusterEnableActionParams alloc] init];

    other.actionID = self.actionID;
    other.invokeID = self.invokeID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRActionsClusterDisableActionParams alloc] init];

    other.actionID = self.actionID;
    other.invokeID = self.invokeID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRBasicClusterMfgSpecificPingParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTROtaSoftwareUpdateProviderClusterQueryImageParams
- (instancetype)init
{
    if (self = [super init]) {

        _vendorId = @(0);

        _productId = @(0);

        _softwareVersion = @(0);

        _protocolsSupported = [NSArray array];

        _hardwareVersion = nil;

        _location = nil;

        _requestorCanConsent = nil;

        _metadataForProvider = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROtaSoftwareUpdateProviderClusterQueryImageParams alloc] init];

    other.vendorId = self.vendorId;
    other.productId = self.productId;
    other.softwareVersion = self.softwareVersion;
    other.protocolsSupported = self.protocolsSupported;
    other.hardwareVersion = self.hardwareVersion;
    other.location = self.location;
    other.requestorCanConsent = self.requestorCanConsent;
    other.metadataForProvider = self.metadataForProvider;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: vendorId:%@; productId:%@; softwareVersion:%@; protocolsSupported:%@; "
                                   @"hardwareVersion:%@; location:%@; requestorCanConsent:%@; metadataForProvider:%@; >",
                  NSStringFromClass([self class]), _vendorId, _productId, _softwareVersion, _protocolsSupported, _hardwareVersion,
                  _location, _requestorCanConsent, [_metadataForProvider base64EncodedStringWithOptions:0]];
    return descriptionString;
}

@end
@implementation MTROtaSoftwareUpdateProviderClusterQueryImageResponseParams
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
    auto other = [[MTROtaSoftwareUpdateProviderClusterQueryImageResponseParams alloc] init];

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

@end
@implementation MTROtaSoftwareUpdateProviderClusterApplyUpdateRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _updateToken = [NSData data];

        _newVersion = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROtaSoftwareUpdateProviderClusterApplyUpdateRequestParams alloc] init];

    other.updateToken = self.updateToken;
    other.newVersion = self.newVersion;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
@implementation MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams
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
    auto other = [[MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams alloc] init];

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

@end
@implementation MTROtaSoftwareUpdateProviderClusterNotifyUpdateAppliedParams
- (instancetype)init
{
    if (self = [super init]) {

        _updateToken = [NSData data];

        _softwareVersion = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROtaSoftwareUpdateProviderClusterNotifyUpdateAppliedParams alloc] init];

    other.updateToken = self.updateToken;
    other.softwareVersion = self.softwareVersion;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
@implementation MTROtaSoftwareUpdateRequestorClusterAnnounceOtaProviderParams
- (instancetype)init
{
    if (self = [super init]) {

        _providerNodeId = @(0);

        _vendorId = @(0);

        _announcementReason = @(0);

        _metadataForNode = nil;

        _endpoint = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROtaSoftwareUpdateRequestorClusterAnnounceOtaProviderParams alloc] init];

    other.providerNodeId = self.providerNodeId;
    other.vendorId = self.vendorId;
    other.announcementReason = self.announcementReason;
    other.metadataForNode = self.metadataForNode;
    other.endpoint = self.endpoint;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:@"<%@: providerNodeId:%@; vendorId:%@; announcementReason:%@; metadataForNode:%@; endpoint:%@; >",
        NSStringFromClass([self class]), _providerNodeId, _vendorId, _announcementReason,
        [_metadataForNode base64EncodedStringWithOptions:0], _endpoint];
    return descriptionString;
}

@end
@implementation MTRGeneralCommissioningClusterArmFailSafeParams
- (instancetype)init
{
    if (self = [super init]) {

        _expiryLengthSeconds = @(0);

        _breadcrumb = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGeneralCommissioningClusterArmFailSafeParams alloc] init];

    other.expiryLengthSeconds = self.expiryLengthSeconds;
    other.breadcrumb = self.breadcrumb;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

@end
@implementation MTRGeneralCommissioningClusterSetRegulatoryConfigParams
- (instancetype)init
{
    if (self = [super init]) {

        _newRegulatoryConfig = @(0);

        _countryCode = @"";

        _breadcrumb = @(0);
        _timedInvokeTimeoutMs = nil;
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

@end
@implementation MTRGeneralCommissioningClusterCommissioningCompleteParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGeneralCommissioningClusterCommissioningCompleteParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

@end
@implementation MTRNetworkCommissioningClusterScanNetworksParams
- (instancetype)init
{
    if (self = [super init]) {

        _ssid = nil;

        _breadcrumb = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRNetworkCommissioningClusterScanNetworksParams alloc] init];

    other.ssid = self.ssid;
    other.breadcrumb = self.breadcrumb;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

@end
@implementation MTRNetworkCommissioningClusterAddOrUpdateWiFiNetworkParams
- (instancetype)init
{
    if (self = [super init]) {

        _ssid = [NSData data];

        _credentials = [NSData data];

        _breadcrumb = nil;
        _timedInvokeTimeoutMs = nil;
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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRNetworkCommissioningClusterAddOrUpdateThreadNetworkParams alloc] init];

    other.operationalDataset = self.operationalDataset;
    other.breadcrumb = self.breadcrumb;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRNetworkCommissioningClusterRemoveNetworkParams alloc] init];

    other.networkID = self.networkID;
    other.breadcrumb = self.breadcrumb;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

@end
@implementation MTRNetworkCommissioningClusterConnectNetworkParams
- (instancetype)init
{
    if (self = [super init]) {

        _networkID = [NSData data];

        _breadcrumb = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRNetworkCommissioningClusterConnectNetworkParams alloc] init];

    other.networkID = self.networkID;
    other.breadcrumb = self.breadcrumb;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

@end
@implementation MTRNetworkCommissioningClusterReorderNetworkParams
- (instancetype)init
{
    if (self = [super init]) {

        _networkID = [NSData data];

        _networkIndex = @(0);

        _breadcrumb = nil;
        _timedInvokeTimeoutMs = nil;
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

        _transferFileDesignator = [NSData data];
        _timedInvokeTimeoutMs = nil;
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

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: intent:%@; requestedProtocol:%@; transferFileDesignator:%@; >",
                                             NSStringFromClass([self class]), _intent, _requestedProtocol,
                                             [_transferFileDesignator base64EncodedStringWithOptions:0]];
    return descriptionString;
}

@end
@implementation MTRDiagnosticLogsClusterRetrieveLogsResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _status = @(0);

        _content = [NSData data];

        _timeStamp = @(0);

        _timeSinceBoot = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDiagnosticLogsClusterRetrieveLogsResponseParams alloc] init];

    other.status = self.status;
    other.content = self.content;
    other.timeStamp = self.timeStamp;
    other.timeSinceBoot = self.timeSinceBoot;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: status:%@; content:%@; timeStamp:%@; timeSinceBoot:%@; >",
                                             NSStringFromClass([self class]), _status, [_content base64EncodedStringWithOptions:0],
                                             _timeStamp, _timeSinceBoot];
    return descriptionString;
}

@end
@implementation MTRGeneralDiagnosticsClusterTestEventTriggerParams
- (instancetype)init
{
    if (self = [super init]) {

        _enableKey = [NSData data];

        _eventTrigger = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGeneralDiagnosticsClusterTestEventTriggerParams alloc] init];

    other.enableKey = self.enableKey;
    other.eventTrigger = self.eventTrigger;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRSoftwareDiagnosticsClusterResetWatermarksParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRThreadNetworkDiagnosticsClusterResetCountsParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRWiFiNetworkDiagnosticsClusterResetCountsParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTREthernetNetworkDiagnosticsClusterResetCountsParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

        _pakeVerifier = [NSData data];

        _discriminator = @(0);

        _iterations = @(0);

        _salt = [NSData data];
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRAdministratorCommissioningClusterOpenCommissioningWindowParams alloc] init];

    other.commissioningTimeout = self.commissioningTimeout;
    other.pakeVerifier = self.pakeVerifier;
    other.discriminator = self.discriminator;
    other.iterations = self.iterations;
    other.salt = self.salt;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: commissioningTimeout:%@; pakeVerifier:%@; discriminator:%@; iterations:%@; salt:%@; >",
                  NSStringFromClass([self class]), _commissioningTimeout, [_pakeVerifier base64EncodedStringWithOptions:0],
                  _discriminator, _iterations, [_salt base64EncodedStringWithOptions:0]];
    return descriptionString;
}

@end
@implementation MTRAdministratorCommissioningClusterOpenBasicCommissioningWindowParams
- (instancetype)init
{
    if (self = [super init]) {

        _commissioningTimeout = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRAdministratorCommissioningClusterOpenBasicCommissioningWindowParams alloc] init];

    other.commissioningTimeout = self.commissioningTimeout;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRAdministratorCommissioningClusterRevokeCommissioningParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROperationalCredentialsClusterAttestationRequestParams alloc] init];

    other.attestationNonce = self.attestationNonce;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

        _signature = [NSData data];
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROperationalCredentialsClusterAttestationResponseParams alloc] init];

    other.attestationElements = self.attestationElements;
    other.signature = self.signature;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: attestationElements:%@; signature:%@; >", NSStringFromClass([self class]),
                  [_attestationElements base64EncodedStringWithOptions:0], [_signature base64EncodedStringWithOptions:0]];
    return descriptionString;
}

@end
@implementation MTROperationalCredentialsClusterCertificateChainRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _certificateType = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROperationalCredentialsClusterCertificateChainRequestParams alloc] init];

    other.certificateType = self.certificateType;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

@end
@implementation MTROperationalCredentialsClusterCSRRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _csrNonce = [NSData data];

        _isForUpdateNOC = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROperationalCredentialsClusterCSRRequestParams alloc] init];

    other.csrNonce = self.csrNonce;
    other.isForUpdateNOC = self.isForUpdateNOC;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROperationalCredentialsClusterUpdateNOCParams alloc] init];

    other.nocValue = self.nocValue;
    other.icacValue = self.icacValue;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

@end
@implementation MTROperationalCredentialsClusterUpdateFabricLabelParams
- (instancetype)init
{
    if (self = [super init]) {

        _label = @"";
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROperationalCredentialsClusterUpdateFabricLabelParams alloc] init];

    other.label = self.label;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROperationalCredentialsClusterRemoveFabricParams alloc] init];

    other.fabricIndex = self.fabricIndex;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

        _rootCertificate = [NSData data];
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTROperationalCredentialsClusterAddTrustedRootCertificateParams alloc] init];

    other.rootCertificate = self.rootCertificate;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: rootCertificate:%@; >", NSStringFromClass([self class]),
                                             [_rootCertificate base64EncodedStringWithOptions:0]];
    return descriptionString;
}

@end
@implementation MTRGroupKeyManagementClusterKeySetWriteParams
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
    auto other = [[MTRGroupKeyManagementClusterKeySetWriteParams alloc] init];

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

@end
@implementation MTRGroupKeyManagementClusterKeySetReadParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupKeySetID = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGroupKeyManagementClusterKeySetReadParams alloc] init];

    other.groupKeySetID = self.groupKeySetID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

@end
@implementation MTRGroupKeyManagementClusterKeySetRemoveParams
- (instancetype)init
{
    if (self = [super init]) {

        _groupKeySetID = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGroupKeyManagementClusterKeySetRemoveParams alloc] init];

    other.groupKeySetID = self.groupKeySetID;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRGroupKeyManagementClusterKeySetReadAllIndicesParams alloc] init];

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

@end
@implementation MTRModeSelectClusterChangeToModeParams
- (instancetype)init
{
    if (self = [super init]) {

        _newMode = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRModeSelectClusterChangeToModeParams alloc] init];

    other.newMode = self.newMode;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: newMode:%@; >", NSStringFromClass([self class]), _newMode];
    return descriptionString;
}

@end
@implementation MTRDoorLockClusterLockDoorParams
- (instancetype)init
{
    if (self = [super init]) {

        _pinCode = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterLockDoorParams alloc] init];

    other.pinCode = self.pinCode;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterUnlockDoorParams alloc] init];

    other.pinCode = self.pinCode;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterUnlockWithTimeoutParams alloc] init];

    other.timeout = self.timeout;
    other.pinCode = self.pinCode;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterGetWeekDayScheduleParams alloc] init];

    other.weekDayIndex = self.weekDayIndex;
    other.userIndex = self.userIndex;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

@end
@implementation MTRDoorLockClusterClearWeekDayScheduleParams
- (instancetype)init
{
    if (self = [super init]) {

        _weekDayIndex = @(0);

        _userIndex = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterClearWeekDayScheduleParams alloc] init];

    other.weekDayIndex = self.weekDayIndex;
    other.userIndex = self.userIndex;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterGetYearDayScheduleParams alloc] init];

    other.yearDayIndex = self.yearDayIndex;
    other.userIndex = self.userIndex;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

@end
@implementation MTRDoorLockClusterClearYearDayScheduleParams
- (instancetype)init
{
    if (self = [super init]) {

        _yearDayIndex = @(0);

        _userIndex = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterClearYearDayScheduleParams alloc] init];

    other.yearDayIndex = self.yearDayIndex;
    other.userIndex = self.userIndex;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterGetHolidayScheduleParams alloc] init];

    other.holidayIndex = self.holidayIndex;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

@end
@implementation MTRDoorLockClusterClearHolidayScheduleParams
- (instancetype)init
{
    if (self = [super init]) {

        _holidayIndex = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterClearHolidayScheduleParams alloc] init];

    other.holidayIndex = self.holidayIndex;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

        _userUniqueId = nil;

        _userStatus = nil;

        _userType = nil;

        _credentialRule = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterSetUserParams alloc] init];

    other.operationType = self.operationType;
    other.userIndex = self.userIndex;
    other.userName = self.userName;
    other.userUniqueId = self.userUniqueId;
    other.userStatus = self.userStatus;
    other.userType = self.userType;
    other.credentialRule = self.credentialRule;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString
        stringWithFormat:
            @"<%@: operationType:%@; userIndex:%@; userName:%@; userUniqueId:%@; userStatus:%@; userType:%@; credentialRule:%@; >",
        NSStringFromClass([self class]), _operationType, _userIndex, _userName, _userUniqueId, _userStatus, _userType,
        _credentialRule];
    return descriptionString;
}

@end
@implementation MTRDoorLockClusterGetUserParams
- (instancetype)init
{
    if (self = [super init]) {

        _userIndex = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterGetUserParams alloc] init];

    other.userIndex = self.userIndex;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

        _userUniqueId = nil;

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
    other.userUniqueId = self.userUniqueId;
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
        stringWithFormat:@"<%@: userIndex:%@; userName:%@; userUniqueId:%@; userStatus:%@; userType:%@; credentialRule:%@; "
                         @"credentials:%@; creatorFabricIndex:%@; lastModifiedFabricIndex:%@; nextUserIndex:%@; >",
        NSStringFromClass([self class]), _userIndex, _userName, _userUniqueId, _userStatus, _userType, _credentialRule,
        _credentials, _creatorFabricIndex, _lastModifiedFabricIndex, _nextUserIndex];
    return descriptionString;
}

@end
@implementation MTRDoorLockClusterClearUserParams
- (instancetype)init
{
    if (self = [super init]) {

        _userIndex = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterClearUserParams alloc] init];

    other.userIndex = self.userIndex;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

        _credential = [MTRDoorLockClusterDlCredential new];

        _credentialData = [NSData data];

        _userIndex = nil;

        _userStatus = nil;

        _userType = nil;
        _timedInvokeTimeoutMs = nil;
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

@end
@implementation MTRDoorLockClusterGetCredentialStatusParams
- (instancetype)init
{
    if (self = [super init]) {

        _credential = [MTRDoorLockClusterDlCredential new];
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterGetCredentialStatusParams alloc] init];

    other.credential = self.credential;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

@end
@implementation MTRDoorLockClusterClearCredentialParams
- (instancetype)init
{
    if (self = [super init]) {

        _credential = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRDoorLockClusterClearCredentialParams alloc] init];

    other.credential = self.credential;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: credential:%@; >", NSStringFromClass([self class]), _credential];
    return descriptionString;
}

@end
@implementation MTRWindowCoveringClusterUpOrOpenParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRWindowCoveringClusterUpOrOpenParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRWindowCoveringClusterDownOrCloseParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRWindowCoveringClusterStopMotionParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRWindowCoveringClusterGoToLiftValueParams alloc] init];

    other.liftValue = self.liftValue;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRWindowCoveringClusterGoToLiftPercentageParams alloc] init];

    other.liftPercent100thsValue = self.liftPercent100thsValue;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRWindowCoveringClusterGoToTiltValueParams alloc] init];

    other.tiltValue = self.tiltValue;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRWindowCoveringClusterGoToTiltPercentageParams alloc] init];

    other.tiltPercent100thsValue = self.tiltPercent100thsValue;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRBarrierControlClusterBarrierControlGoToPercentParams alloc] init];

    other.percentOpen = self.percentOpen;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRBarrierControlClusterBarrierControlStopParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRThermostatClusterSetpointRaiseLowerParams alloc] init];

    other.mode = self.mode;
    other.amount = self.amount;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRThermostatClusterGetWeeklyScheduleParams alloc] init];

    other.daysToReturn = self.daysToReturn;
    other.modeToReturn = self.modeToReturn;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRThermostatClusterClearWeeklyScheduleParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
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

        _colorTemperature = @(0);

        _transitionTime = @(0);

        _optionsMask = @(0);

        _optionsOverride = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRColorControlClusterMoveToColorTemperatureParams alloc] init];

    other.colorTemperature = self.colorTemperature;
    other.transitionTime = self.transitionTime;
    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: colorTemperature:%@; transitionTime:%@; optionsMask:%@; optionsOverride:%@; >",
                  NSStringFromClass([self class]), _colorTemperature, _transitionTime, _optionsMask, _optionsOverride];
    return descriptionString;
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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRColorControlClusterStopMoveStepParams alloc] init];

    other.optionsMask = self.optionsMask;
    other.optionsOverride = self.optionsOverride;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRChannelClusterChangeChannelParams alloc] init];

    other.match = self.match;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

@end
@implementation MTRChannelClusterChangeChannelByNumberParams
- (instancetype)init
{
    if (self = [super init]) {

        _majorNumber = @(0);

        _minorNumber = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRChannelClusterChangeChannelByNumberParams alloc] init];

    other.majorNumber = self.majorNumber;
    other.minorNumber = self.minorNumber;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRChannelClusterSkipChannelParams alloc] init];

    other.count = self.count;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRTargetNavigatorClusterNavigateTargetParams alloc] init];

    other.target = self.target;
    other.data = self.data;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

@end
@implementation MTRMediaPlaybackClusterPlayParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaPlaybackClusterPlayParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaPlaybackClusterPauseParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRMediaPlaybackClusterStopPlaybackParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaPlaybackClusterStopPlaybackParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRMediaPlaybackClusterStartOverParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaPlaybackClusterStartOverParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaPlaybackClusterPreviousParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaPlaybackClusterNextParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaPlaybackClusterRewindParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaPlaybackClusterFastForwardParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaPlaybackClusterSkipForwardParams alloc] init];

    other.deltaPositionMilliseconds = self.deltaPositionMilliseconds;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaPlaybackClusterSkipBackwardParams alloc] init];

    other.deltaPositionMilliseconds = self.deltaPositionMilliseconds;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

@end
@implementation MTRMediaPlaybackClusterSeekParams
- (instancetype)init
{
    if (self = [super init]) {

        _position = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaPlaybackClusterSeekParams alloc] init];

    other.position = self.position;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaInputClusterSelectInputParams alloc] init];

    other.index = self.index;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaInputClusterShowInputStatusParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaInputClusterHideInputStatusParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRMediaInputClusterRenameInputParams alloc] init];

    other.index = self.index;
    other.name = self.name;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRLowPowerClusterSleepParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRKeypadInputClusterSendKeyParams alloc] init];

    other.keyCode = self.keyCode;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

@end
@implementation MTRContentLauncherClusterLaunchContentParams
- (instancetype)init
{
    if (self = [super init]) {

        _search = [MTRContentLauncherClusterContentSearch new];

        _autoPlay = @(0);

        _data = nil;
        _timedInvokeTimeoutMs = nil;
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

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: contentURL:%@; displayString:%@; brandingInformation:%@; >",
                                             NSStringFromClass([self class]), _contentURL, _displayString, _brandingInformation];
    return descriptionString;
}

@end
@implementation MTRContentLauncherClusterLaunchResponseParams
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
    auto other = [[MTRContentLauncherClusterLaunchResponseParams alloc] init];

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

@end
@implementation MTRAudioOutputClusterSelectOutputParams
- (instancetype)init
{
    if (self = [super init]) {

        _index = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRAudioOutputClusterSelectOutputParams alloc] init];

    other.index = self.index;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRAudioOutputClusterRenameOutputParams alloc] init];

    other.index = self.index;
    other.name = self.name;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

        _application = [MTRApplicationLauncherClusterApplication new];

        _data = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRApplicationLauncherClusterLaunchAppParams alloc] init];

    other.application = self.application;
    other.data = self.data;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

        _application = [MTRApplicationLauncherClusterApplication new];
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRApplicationLauncherClusterStopAppParams alloc] init];

    other.application = self.application;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

        _application = [MTRApplicationLauncherClusterApplication new];
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRApplicationLauncherClusterHideAppParams alloc] init];

    other.application = self.application;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

        _data = [NSData data];
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

@end
@implementation MTRAccountLoginClusterGetSetupPINParams
- (instancetype)init
{
    if (self = [super init]) {

        _tempAccountIdentifier = @"";
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRAccountLoginClusterGetSetupPINParams alloc] init];

    other.tempAccountIdentifier = self.tempAccountIdentifier;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

@end
@implementation MTRAccountLoginClusterLoginParams
- (instancetype)init
{
    if (self = [super init]) {

        _tempAccountIdentifier = @"";

        _setupPIN = @"";
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRAccountLoginClusterLoginParams alloc] init];

    other.tempAccountIdentifier = self.tempAccountIdentifier;
    other.setupPIN = self.setupPIN;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRAccountLoginClusterLogoutParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

@end
@implementation MTRElectricalMeasurementClusterGetProfileInfoCommandParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRElectricalMeasurementClusterGetProfileInfoCommandParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

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

@end
@implementation MTRElectricalMeasurementClusterGetMeasurementProfileCommandParams
- (instancetype)init
{
    if (self = [super init]) {

        _attributeId = @(0);

        _startTime = @(0);

        _numberOfIntervals = @(0);
        _timedInvokeTimeoutMs = nil;
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

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: attributeId:%@; startTime:%@; numberOfIntervals:%@; >",
                                             NSStringFromClass([self class]), _attributeId, _startTime, _numberOfIntervals];
    return descriptionString;
}

@end
@implementation MTRTestClusterClusterTestParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRTestClusterClusterTestParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRTestClusterClusterTestSpecificResponseParams
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
    auto other = [[MTRTestClusterClusterTestSpecificResponseParams alloc] init];

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

@end
@implementation MTRTestClusterClusterTestNotHandledParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRTestClusterClusterTestNotHandledParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRTestClusterClusterTestAddArgumentsResponseParams
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
    auto other = [[MTRTestClusterClusterTestAddArgumentsResponseParams alloc] init];

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

@end
@implementation MTRTestClusterClusterTestSpecificParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRTestClusterClusterTestSpecificParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRTestClusterClusterTestSimpleArgumentResponseParams
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
    auto other = [[MTRTestClusterClusterTestSimpleArgumentResponseParams alloc] init];

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

@end
@implementation MTRTestClusterClusterTestUnknownCommandParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRTestClusterClusterTestUnknownCommandParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRTestClusterClusterTestStructArrayArgumentResponseParams
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
    auto other = [[MTRTestClusterClusterTestStructArrayArgumentResponseParams alloc] init];

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

@end
@implementation MTRTestClusterClusterTestAddArgumentsParams
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
    auto other = [[MTRTestClusterClusterTestAddArgumentsParams alloc] init];

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

@end
@implementation MTRTestClusterClusterTestListInt8UReverseResponseParams
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
    auto other = [[MTRTestClusterClusterTestListInt8UReverseResponseParams alloc] init];

    other.arg1 = self.arg1;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; >", NSStringFromClass([self class]), _arg1];
    return descriptionString;
}

@end
@implementation MTRTestClusterClusterTestSimpleArgumentRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRTestClusterClusterTestSimpleArgumentRequestParams alloc] init];

    other.arg1 = self.arg1;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; >", NSStringFromClass([self class]), _arg1];
    return descriptionString;
}

@end
@implementation MTRTestClusterClusterTestEnumsResponseParams
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
    auto other = [[MTRTestClusterClusterTestEnumsResponseParams alloc] init];

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

@end
@implementation MTRTestClusterClusterTestStructArrayArgumentRequestParams
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
    auto other = [[MTRTestClusterClusterTestStructArrayArgumentRequestParams alloc] init];

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

@end
@implementation MTRTestClusterClusterTestNullableOptionalResponseParams
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
    auto other = [[MTRTestClusterClusterTestNullableOptionalResponseParams alloc] init];

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

@end
@implementation MTRTestClusterClusterTestStructArgumentRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = [MTRTestClusterClusterSimpleStruct new];
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRTestClusterClusterTestStructArgumentRequestParams alloc] init];

    other.arg1 = self.arg1;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; >", NSStringFromClass([self class]), _arg1];
    return descriptionString;
}

@end
@implementation MTRTestClusterClusterTestComplexNullableOptionalResponseParams
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
    auto other = [[MTRTestClusterClusterTestComplexNullableOptionalResponseParams alloc] init];

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

@end
@implementation MTRTestClusterClusterTestNestedStructArgumentRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = [MTRTestClusterClusterNestedStruct new];
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRTestClusterClusterTestNestedStructArgumentRequestParams alloc] init];

    other.arg1 = self.arg1;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; >", NSStringFromClass([self class]), _arg1];
    return descriptionString;
}

@end
@implementation MTRTestClusterClusterBooleanResponseParams
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
    auto other = [[MTRTestClusterClusterBooleanResponseParams alloc] init];

    other.value = self.value;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: value:%@; >", NSStringFromClass([self class]), _value];
    return descriptionString;
}

@end
@implementation MTRTestClusterClusterTestListStructArgumentRequestParams
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
    auto other = [[MTRTestClusterClusterTestListStructArgumentRequestParams alloc] init];

    other.arg1 = self.arg1;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; >", NSStringFromClass([self class]), _arg1];
    return descriptionString;
}

@end
@implementation MTRTestClusterClusterSimpleStructResponseParams
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = [MTRTestClusterClusterSimpleStruct new];
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRTestClusterClusterSimpleStructResponseParams alloc] init];

    other.arg1 = self.arg1;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; >", NSStringFromClass([self class]), _arg1];
    return descriptionString;
}

@end
@implementation MTRTestClusterClusterTestListInt8UArgumentRequestParams
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
    auto other = [[MTRTestClusterClusterTestListInt8UArgumentRequestParams alloc] init];

    other.arg1 = self.arg1;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; >", NSStringFromClass([self class]), _arg1];
    return descriptionString;
}

@end
@implementation MTRTestClusterClusterTestEmitTestEventResponseParams
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
    auto other = [[MTRTestClusterClusterTestEmitTestEventResponseParams alloc] init];

    other.value = self.value;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: value:%@; >", NSStringFromClass([self class]), _value];
    return descriptionString;
}

@end
@implementation MTRTestClusterClusterTestNestedStructListArgumentRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = [MTRTestClusterClusterNestedStructList new];
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRTestClusterClusterTestNestedStructListArgumentRequestParams alloc] init];

    other.arg1 = self.arg1;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; >", NSStringFromClass([self class]), _arg1];
    return descriptionString;
}

@end
@implementation MTRTestClusterClusterTestEmitTestFabricScopedEventResponseParams
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
    auto other = [[MTRTestClusterClusterTestEmitTestFabricScopedEventResponseParams alloc] init];

    other.value = self.value;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: value:%@; >", NSStringFromClass([self class]), _value];
    return descriptionString;
}

@end
@implementation MTRTestClusterClusterTestListNestedStructListArgumentRequestParams
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
    auto other = [[MTRTestClusterClusterTestListNestedStructListArgumentRequestParams alloc] init];

    other.arg1 = self.arg1;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; >", NSStringFromClass([self class]), _arg1];
    return descriptionString;
}

@end
@implementation MTRTestClusterClusterTestListInt8UReverseRequestParams
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
    auto other = [[MTRTestClusterClusterTestListInt8UReverseRequestParams alloc] init];

    other.arg1 = self.arg1;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; >", NSStringFromClass([self class]), _arg1];
    return descriptionString;
}

@end
@implementation MTRTestClusterClusterTestEnumsRequestParams
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
    auto other = [[MTRTestClusterClusterTestEnumsRequestParams alloc] init];

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

@end
@implementation MTRTestClusterClusterTestNullableOptionalRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRTestClusterClusterTestNullableOptionalRequestParams alloc] init];

    other.arg1 = self.arg1;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; >", NSStringFromClass([self class]), _arg1];
    return descriptionString;
}

@end
@implementation MTRTestClusterClusterTestComplexNullableOptionalRequestParams
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
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRTestClusterClusterTestComplexNullableOptionalRequestParams alloc] init];

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
@implementation MTRTestClusterClusterSimpleStructEchoRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = [MTRTestClusterClusterSimpleStruct new];
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRTestClusterClusterSimpleStructEchoRequestParams alloc] init];

    other.arg1 = self.arg1;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; >", NSStringFromClass([self class]), _arg1];
    return descriptionString;
}

@end
@implementation MTRTestClusterClusterTimedInvokeRequestParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRTestClusterClusterTimedInvokeRequestParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end
@implementation MTRTestClusterClusterTestSimpleOptionalArgumentRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = nil;
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRTestClusterClusterTestSimpleOptionalArgumentRequestParams alloc] init];

    other.arg1 = self.arg1;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; >", NSStringFromClass([self class]), _arg1];
    return descriptionString;
}

@end
@implementation MTRTestClusterClusterTestEmitTestEventRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = @(0);

        _arg2 = @(0);

        _arg3 = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRTestClusterClusterTestEmitTestEventRequestParams alloc] init];

    other.arg1 = self.arg1;
    other.arg2 = self.arg2;
    other.arg3 = self.arg3;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString =
        [NSString stringWithFormat:@"<%@: arg1:%@; arg2:%@; arg3:%@; >", NSStringFromClass([self class]), _arg1, _arg2, _arg3];
    return descriptionString;
}

@end
@implementation MTRTestClusterClusterTestEmitTestFabricScopedEventRequestParams
- (instancetype)init
{
    if (self = [super init]) {

        _arg1 = @(0);
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRTestClusterClusterTestEmitTestFabricScopedEventRequestParams alloc] init];

    other.arg1 = self.arg1;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: arg1:%@; >", NSStringFromClass([self class]), _arg1];
    return descriptionString;
}

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
