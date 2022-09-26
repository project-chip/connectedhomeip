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
#import <Matter/MTRStructsObjc.h>

NS_ASSUME_NONNULL_BEGIN

@interface MTRIdentifyClusterIdentifyParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull identifyTime;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRIdentifyClusterTriggerEffectParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull effectIdentifier;

@property (nonatomic, copy) NSNumber * _Nonnull effectVariant;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRGroupsClusterAddGroupParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupId;

@property (nonatomic, copy) NSString * _Nonnull groupName;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRGroupsClusterAddGroupResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status;

@property (nonatomic, copy) NSNumber * _Nonnull groupId;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRGroupsClusterViewGroupParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupId;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRGroupsClusterViewGroupResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status;

@property (nonatomic, copy) NSNumber * _Nonnull groupId;

@property (nonatomic, copy) NSString * _Nonnull groupName;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRGroupsClusterGetGroupMembershipParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull groupList;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRGroupsClusterGetGroupMembershipResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nullable capacity;

@property (nonatomic, copy) NSArray * _Nonnull groupList;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRGroupsClusterRemoveGroupParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupId;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRGroupsClusterRemoveGroupResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status;

@property (nonatomic, copy) NSNumber * _Nonnull groupId;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRGroupsClusterRemoveAllGroupsParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRGroupsClusterAddGroupIfIdentifyingParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupId;

@property (nonatomic, copy) NSString * _Nonnull groupName;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRScenesClusterAddSceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupId;

@property (nonatomic, copy) NSNumber * _Nonnull sceneId;

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime;

@property (nonatomic, copy) NSString * _Nonnull sceneName;

@property (nonatomic, copy) NSArray * _Nonnull extensionFieldSets;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRScenesClusterAddSceneResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status;

@property (nonatomic, copy) NSNumber * _Nonnull groupId;

@property (nonatomic, copy) NSNumber * _Nonnull sceneId;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRScenesClusterViewSceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupId;

@property (nonatomic, copy) NSNumber * _Nonnull sceneId;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRScenesClusterViewSceneResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status;

@property (nonatomic, copy) NSNumber * _Nonnull groupId;

@property (nonatomic, copy) NSNumber * _Nonnull sceneId;

@property (nonatomic, copy) NSNumber * _Nullable transitionTime;

@property (nonatomic, copy) NSString * _Nullable sceneName;

@property (nonatomic, copy) NSArray * _Nullable extensionFieldSets;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRScenesClusterRemoveSceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupId;

@property (nonatomic, copy) NSNumber * _Nonnull sceneId;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRScenesClusterRemoveSceneResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status;

@property (nonatomic, copy) NSNumber * _Nonnull groupId;

@property (nonatomic, copy) NSNumber * _Nonnull sceneId;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRScenesClusterRemoveAllScenesParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupId;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRScenesClusterRemoveAllScenesResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status;

@property (nonatomic, copy) NSNumber * _Nonnull groupId;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRScenesClusterStoreSceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupId;

@property (nonatomic, copy) NSNumber * _Nonnull sceneId;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRScenesClusterStoreSceneResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status;

@property (nonatomic, copy) NSNumber * _Nonnull groupId;

@property (nonatomic, copy) NSNumber * _Nonnull sceneId;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRScenesClusterRecallSceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupId;

@property (nonatomic, copy) NSNumber * _Nonnull sceneId;

@property (nonatomic, copy) NSNumber * _Nullable transitionTime;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRScenesClusterGetSceneMembershipParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupId;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRScenesClusterGetSceneMembershipResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status;

@property (nonatomic, copy) NSNumber * _Nullable capacity;

@property (nonatomic, copy) NSNumber * _Nonnull groupId;

@property (nonatomic, copy) NSArray * _Nullable sceneList;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRScenesClusterEnhancedAddSceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupId;

@property (nonatomic, copy) NSNumber * _Nonnull sceneId;

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime;

@property (nonatomic, copy) NSString * _Nonnull sceneName;

@property (nonatomic, copy) NSArray * _Nonnull extensionFieldSets;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRScenesClusterEnhancedAddSceneResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status;

@property (nonatomic, copy) NSNumber * _Nonnull groupId;

@property (nonatomic, copy) NSNumber * _Nonnull sceneId;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRScenesClusterEnhancedViewSceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupId;

@property (nonatomic, copy) NSNumber * _Nonnull sceneId;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRScenesClusterEnhancedViewSceneResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status;

@property (nonatomic, copy) NSNumber * _Nonnull groupId;

@property (nonatomic, copy) NSNumber * _Nonnull sceneId;

@property (nonatomic, copy) NSNumber * _Nullable transitionTime;

@property (nonatomic, copy) NSString * _Nullable sceneName;

@property (nonatomic, copy) NSArray * _Nullable extensionFieldSets;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRScenesClusterCopySceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull mode;

@property (nonatomic, copy) NSNumber * _Nonnull groupIdFrom;

@property (nonatomic, copy) NSNumber * _Nonnull sceneIdFrom;

@property (nonatomic, copy) NSNumber * _Nonnull groupIdTo;

@property (nonatomic, copy) NSNumber * _Nonnull sceneIdTo;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRScenesClusterCopySceneResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status;

@property (nonatomic, copy) NSNumber * _Nonnull groupIdFrom;

@property (nonatomic, copy) NSNumber * _Nonnull sceneIdFrom;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTROnOffClusterOffParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTROnOffClusterOnParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTROnOffClusterToggleParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTROnOffClusterOffWithEffectParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull effectId;

@property (nonatomic, copy) NSNumber * _Nonnull effectVariant;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTROnOffClusterOnWithRecallGlobalSceneParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTROnOffClusterOnWithTimedOffParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull onOffControl;

@property (nonatomic, copy) NSNumber * _Nonnull onTime;

@property (nonatomic, copy) NSNumber * _Nonnull offWaitTime;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRLevelControlClusterMoveToLevelParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull level;

@property (nonatomic, copy) NSNumber * _Nullable transitionTime;

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask;

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRLevelControlClusterMoveParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull moveMode;

@property (nonatomic, copy) NSNumber * _Nullable rate;

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask;

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRLevelControlClusterStepParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull stepMode;

@property (nonatomic, copy) NSNumber * _Nonnull stepSize;

@property (nonatomic, copy) NSNumber * _Nullable transitionTime;

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask;

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRLevelControlClusterStopParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask;

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRLevelControlClusterMoveToLevelWithOnOffParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull level;

@property (nonatomic, copy) NSNumber * _Nullable transitionTime;

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask;

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRLevelControlClusterMoveWithOnOffParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull moveMode;

@property (nonatomic, copy) NSNumber * _Nullable rate;

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask;

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRLevelControlClusterStepWithOnOffParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull stepMode;

@property (nonatomic, copy) NSNumber * _Nonnull stepSize;

@property (nonatomic, copy) NSNumber * _Nullable transitionTime;

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask;

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRLevelControlClusterStopWithOnOffParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask;

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRLevelControlClusterMoveToClosestFrequencyParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull frequency;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRActionsClusterInstantActionParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID;

@property (nonatomic, copy) NSNumber * _Nullable invokeID;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRActionsClusterInstantActionWithTransitionParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID;

@property (nonatomic, copy) NSNumber * _Nullable invokeID;

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRActionsClusterStartActionParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID;

@property (nonatomic, copy) NSNumber * _Nullable invokeID;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRActionsClusterStartActionWithDurationParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID;

@property (nonatomic, copy) NSNumber * _Nullable invokeID;

@property (nonatomic, copy) NSNumber * _Nonnull duration;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRActionsClusterStopActionParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID;

@property (nonatomic, copy) NSNumber * _Nullable invokeID;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRActionsClusterPauseActionParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID;

@property (nonatomic, copy) NSNumber * _Nullable invokeID;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRActionsClusterPauseActionWithDurationParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID;

@property (nonatomic, copy) NSNumber * _Nullable invokeID;

@property (nonatomic, copy) NSNumber * _Nonnull duration;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRActionsClusterResumeActionParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID;

@property (nonatomic, copy) NSNumber * _Nullable invokeID;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRActionsClusterEnableActionParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID;

@property (nonatomic, copy) NSNumber * _Nullable invokeID;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRActionsClusterEnableActionWithDurationParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID;

@property (nonatomic, copy) NSNumber * _Nullable invokeID;

@property (nonatomic, copy) NSNumber * _Nonnull duration;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRActionsClusterDisableActionParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID;

@property (nonatomic, copy) NSNumber * _Nullable invokeID;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRActionsClusterDisableActionWithDurationParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID;

@property (nonatomic, copy) NSNumber * _Nullable invokeID;

@property (nonatomic, copy) NSNumber * _Nonnull duration;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRBasicClusterMfgSpecificPingParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTROTASoftwareUpdateProviderClusterQueryImageParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull vendorId;

@property (nonatomic, copy) NSNumber * _Nonnull productId;

@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion;

@property (nonatomic, copy) NSArray * _Nonnull protocolsSupported;

@property (nonatomic, copy) NSNumber * _Nullable hardwareVersion;

@property (nonatomic, copy) NSString * _Nullable location;

@property (nonatomic, copy) NSNumber * _Nullable requestorCanConsent;

@property (nonatomic, copy) NSData * _Nullable metadataForProvider;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTROTASoftwareUpdateProviderClusterQueryImageResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status;

@property (nonatomic, copy) NSNumber * _Nullable delayedActionTime;

@property (nonatomic, copy) NSString * _Nullable imageURI;

@property (nonatomic, copy) NSNumber * _Nullable softwareVersion;

@property (nonatomic, copy) NSString * _Nullable softwareVersionString;

@property (nonatomic, copy) NSData * _Nullable updateToken;

@property (nonatomic, copy) NSNumber * _Nullable userConsentNeeded;

@property (nonatomic, copy) NSData * _Nullable metadataForRequestor;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTROTASoftwareUpdateProviderClusterApplyUpdateRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull updateToken;

@property (nonatomic, copy, getter=getNewVersion) NSNumber * _Nonnull newVersion;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull action;

@property (nonatomic, copy) NSNumber * _Nonnull delayedActionTime;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTROTASoftwareUpdateProviderClusterNotifyUpdateAppliedParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull updateToken;

@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTROTASoftwareUpdateRequestorClusterAnnounceOtaProviderParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull providerNodeId;

@property (nonatomic, copy) NSNumber * _Nonnull vendorId;

@property (nonatomic, copy) NSNumber * _Nonnull announcementReason;

@property (nonatomic, copy) NSData * _Nullable metadataForNode;

@property (nonatomic, copy) NSNumber * _Nonnull endpoint;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRGeneralCommissioningClusterArmFailSafeParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull expiryLengthSeconds;

@property (nonatomic, copy) NSNumber * _Nonnull breadcrumb;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRGeneralCommissioningClusterArmFailSafeResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull errorCode;

@property (nonatomic, copy) NSString * _Nonnull debugText;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRGeneralCommissioningClusterSetRegulatoryConfigParams : NSObject <NSCopying>

@property (nonatomic, copy, getter=getNewRegulatoryConfig) NSNumber * _Nonnull newRegulatoryConfig;

@property (nonatomic, copy) NSString * _Nonnull countryCode;

@property (nonatomic, copy) NSNumber * _Nonnull breadcrumb;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRGeneralCommissioningClusterSetRegulatoryConfigResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull errorCode;

@property (nonatomic, copy) NSString * _Nonnull debugText;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRGeneralCommissioningClusterCommissioningCompleteParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRGeneralCommissioningClusterCommissioningCompleteResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull errorCode;

@property (nonatomic, copy) NSString * _Nonnull debugText;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRNetworkCommissioningClusterScanNetworksParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nullable ssid;

@property (nonatomic, copy) NSNumber * _Nullable breadcrumb;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRNetworkCommissioningClusterScanNetworksResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull networkingStatus;

@property (nonatomic, copy) NSString * _Nullable debugText;

@property (nonatomic, copy) NSArray * _Nullable wiFiScanResults;

@property (nonatomic, copy) NSArray * _Nullable threadScanResults;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRNetworkCommissioningClusterAddOrUpdateWiFiNetworkParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull ssid;

@property (nonatomic, copy) NSData * _Nonnull credentials;

@property (nonatomic, copy) NSNumber * _Nullable breadcrumb;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRNetworkCommissioningClusterAddOrUpdateThreadNetworkParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull operationalDataset;

@property (nonatomic, copy) NSNumber * _Nullable breadcrumb;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRNetworkCommissioningClusterRemoveNetworkParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull networkID;

@property (nonatomic, copy) NSNumber * _Nullable breadcrumb;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRNetworkCommissioningClusterNetworkConfigResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull networkingStatus;

@property (nonatomic, copy) NSString * _Nullable debugText;

@property (nonatomic, copy) NSNumber * _Nullable networkIndex;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRNetworkCommissioningClusterConnectNetworkParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull networkID;

@property (nonatomic, copy) NSNumber * _Nullable breadcrumb;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRNetworkCommissioningClusterConnectNetworkResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull networkingStatus;

@property (nonatomic, copy) NSString * _Nullable debugText;

@property (nonatomic, copy) NSNumber * _Nullable errorValue;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRNetworkCommissioningClusterReorderNetworkParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull networkID;

@property (nonatomic, copy) NSNumber * _Nonnull networkIndex;

@property (nonatomic, copy) NSNumber * _Nullable breadcrumb;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRDiagnosticLogsClusterRetrieveLogsRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull intent;

@property (nonatomic, copy) NSNumber * _Nonnull requestedProtocol;

@property (nonatomic, copy) NSData * _Nonnull transferFileDesignator;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRDiagnosticLogsClusterRetrieveLogsResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status;

@property (nonatomic, copy) NSData * _Nonnull content;

@property (nonatomic, copy) NSNumber * _Nonnull timeStamp;

@property (nonatomic, copy) NSNumber * _Nonnull timeSinceBoot;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRGeneralDiagnosticsClusterTestEventTriggerParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull enableKey;

@property (nonatomic, copy) NSNumber * _Nonnull eventTrigger;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRSoftwareDiagnosticsClusterResetWatermarksParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRThreadNetworkDiagnosticsClusterResetCountsParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRWiFiNetworkDiagnosticsClusterResetCountsParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTREthernetNetworkDiagnosticsClusterResetCountsParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTimeSynchronizationClusterSetUtcTimeParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull utcTime;

@property (nonatomic, copy) NSNumber * _Nonnull granularity;

@property (nonatomic, copy) NSNumber * _Nullable timeSource;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRAdministratorCommissioningClusterOpenCommissioningWindowParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull commissioningTimeout;

@property (nonatomic, copy) NSData * _Nonnull pakeVerifier;

@property (nonatomic, copy) NSNumber * _Nonnull discriminator;

@property (nonatomic, copy) NSNumber * _Nonnull iterations;

@property (nonatomic, copy) NSData * _Nonnull salt;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRAdministratorCommissioningClusterOpenBasicCommissioningWindowParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull commissioningTimeout;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRAdministratorCommissioningClusterRevokeCommissioningParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTROperationalCredentialsClusterAttestationRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull attestationNonce;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTROperationalCredentialsClusterAttestationResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull attestationElements;

@property (nonatomic, copy) NSData * _Nonnull signature;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTROperationalCredentialsClusterCertificateChainRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull certificateType;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTROperationalCredentialsClusterCertificateChainResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull certificate;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTROperationalCredentialsClusterCSRRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull csrNonce;

@property (nonatomic, copy) NSNumber * _Nullable isForUpdateNOC;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTROperationalCredentialsClusterCSRResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull nocsrElements;

@property (nonatomic, copy) NSData * _Nonnull attestationSignature;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTROperationalCredentialsClusterAddNOCParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull nocValue;

@property (nonatomic, copy) NSData * _Nullable icacValue;

@property (nonatomic, copy) NSData * _Nonnull ipkValue;

@property (nonatomic, copy) NSNumber * _Nonnull caseAdminSubject;

@property (nonatomic, copy) NSNumber * _Nonnull adminVendorId;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTROperationalCredentialsClusterUpdateNOCParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull nocValue;

@property (nonatomic, copy) NSData * _Nullable icacValue;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTROperationalCredentialsClusterNOCResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull statusCode;

@property (nonatomic, copy) NSNumber * _Nullable fabricIndex;

@property (nonatomic, copy) NSString * _Nullable debugText;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTROperationalCredentialsClusterUpdateFabricLabelParams : NSObject <NSCopying>

@property (nonatomic, copy) NSString * _Nonnull label;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTROperationalCredentialsClusterRemoveFabricParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTROperationalCredentialsClusterAddTrustedRootCertificateParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull rootCertificate;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRGroupKeyManagementClusterKeySetWriteParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRGroupKeyManagementClusterGroupKeySetStruct * _Nonnull groupKeySet;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRGroupKeyManagementClusterKeySetReadParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupKeySetID;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRGroupKeyManagementClusterKeySetReadResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRGroupKeyManagementClusterGroupKeySetStruct * _Nonnull groupKeySet;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRGroupKeyManagementClusterKeySetRemoveParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupKeySetID;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRGroupKeyManagementClusterKeySetReadAllIndicesParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull groupKeySetIDs;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRGroupKeyManagementClusterKeySetReadAllIndicesResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull groupKeySetIDs;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRModeSelectClusterChangeToModeParams : NSObject <NSCopying>

@property (nonatomic, copy, getter=getNewMode) NSNumber * _Nonnull newMode;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRDoorLockClusterLockDoorParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nullable pinCode;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRDoorLockClusterUnlockDoorParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nullable pinCode;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRDoorLockClusterUnlockWithTimeoutParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull timeout;

@property (nonatomic, copy) NSData * _Nullable pinCode;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRDoorLockClusterSetWeekDayScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull weekDayIndex;

@property (nonatomic, copy) NSNumber * _Nonnull userIndex;

@property (nonatomic, copy) NSNumber * _Nonnull daysMask;

@property (nonatomic, copy) NSNumber * _Nonnull startHour;

@property (nonatomic, copy) NSNumber * _Nonnull startMinute;

@property (nonatomic, copy) NSNumber * _Nonnull endHour;

@property (nonatomic, copy) NSNumber * _Nonnull endMinute;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRDoorLockClusterGetWeekDayScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull weekDayIndex;

@property (nonatomic, copy) NSNumber * _Nonnull userIndex;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRDoorLockClusterGetWeekDayScheduleResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull weekDayIndex;

@property (nonatomic, copy) NSNumber * _Nonnull userIndex;

@property (nonatomic, copy) NSNumber * _Nonnull status;

@property (nonatomic, copy) NSNumber * _Nullable daysMask;

@property (nonatomic, copy) NSNumber * _Nullable startHour;

@property (nonatomic, copy) NSNumber * _Nullable startMinute;

@property (nonatomic, copy) NSNumber * _Nullable endHour;

@property (nonatomic, copy) NSNumber * _Nullable endMinute;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRDoorLockClusterClearWeekDayScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull weekDayIndex;

@property (nonatomic, copy) NSNumber * _Nonnull userIndex;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRDoorLockClusterSetYearDayScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull yearDayIndex;

@property (nonatomic, copy) NSNumber * _Nonnull userIndex;

@property (nonatomic, copy) NSNumber * _Nonnull localStartTime;

@property (nonatomic, copy) NSNumber * _Nonnull localEndTime;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRDoorLockClusterGetYearDayScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull yearDayIndex;

@property (nonatomic, copy) NSNumber * _Nonnull userIndex;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRDoorLockClusterGetYearDayScheduleResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull yearDayIndex;

@property (nonatomic, copy) NSNumber * _Nonnull userIndex;

@property (nonatomic, copy) NSNumber * _Nonnull status;

@property (nonatomic, copy) NSNumber * _Nullable localStartTime;

@property (nonatomic, copy) NSNumber * _Nullable localEndTime;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRDoorLockClusterClearYearDayScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull yearDayIndex;

@property (nonatomic, copy) NSNumber * _Nonnull userIndex;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRDoorLockClusterSetHolidayScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull holidayIndex;

@property (nonatomic, copy) NSNumber * _Nonnull localStartTime;

@property (nonatomic, copy) NSNumber * _Nonnull localEndTime;

@property (nonatomic, copy) NSNumber * _Nonnull operatingMode;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRDoorLockClusterGetHolidayScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull holidayIndex;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRDoorLockClusterGetHolidayScheduleResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull holidayIndex;

@property (nonatomic, copy) NSNumber * _Nonnull status;

@property (nonatomic, copy) NSNumber * _Nullable localStartTime;

@property (nonatomic, copy) NSNumber * _Nullable localEndTime;

@property (nonatomic, copy) NSNumber * _Nullable operatingMode;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRDoorLockClusterClearHolidayScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull holidayIndex;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRDoorLockClusterSetUserParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull operationType;

@property (nonatomic, copy) NSNumber * _Nonnull userIndex;

@property (nonatomic, copy) NSString * _Nullable userName;

@property (nonatomic, copy) NSNumber * _Nullable userUniqueId;

@property (nonatomic, copy) NSNumber * _Nullable userStatus;

@property (nonatomic, copy) NSNumber * _Nullable userType;

@property (nonatomic, copy) NSNumber * _Nullable credentialRule;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRDoorLockClusterGetUserParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull userIndex;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRDoorLockClusterGetUserResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull userIndex;

@property (nonatomic, copy) NSString * _Nullable userName;

@property (nonatomic, copy) NSNumber * _Nullable userUniqueId;

@property (nonatomic, copy) NSNumber * _Nullable userStatus;

@property (nonatomic, copy) NSNumber * _Nullable userType;

@property (nonatomic, copy) NSNumber * _Nullable credentialRule;

@property (nonatomic, copy) NSArray * _Nullable credentials;

@property (nonatomic, copy) NSNumber * _Nullable creatorFabricIndex;

@property (nonatomic, copy) NSNumber * _Nullable lastModifiedFabricIndex;

@property (nonatomic, copy) NSNumber * _Nullable nextUserIndex;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRDoorLockClusterClearUserParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull userIndex;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRDoorLockClusterSetCredentialParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull operationType;

@property (nonatomic, copy) MTRDoorLockClusterDlCredential * _Nonnull credential;

@property (nonatomic, copy) NSData * _Nonnull credentialData;

@property (nonatomic, copy) NSNumber * _Nullable userIndex;

@property (nonatomic, copy) NSNumber * _Nullable userStatus;

@property (nonatomic, copy) NSNumber * _Nullable userType;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRDoorLockClusterSetCredentialResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status;

@property (nonatomic, copy) NSNumber * _Nullable userIndex;

@property (nonatomic, copy) NSNumber * _Nullable nextCredentialIndex;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRDoorLockClusterGetCredentialStatusParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRDoorLockClusterDlCredential * _Nonnull credential;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRDoorLockClusterGetCredentialStatusResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull credentialExists;

@property (nonatomic, copy) NSNumber * _Nullable userIndex;

@property (nonatomic, copy) NSNumber * _Nullable creatorFabricIndex;

@property (nonatomic, copy) NSNumber * _Nullable lastModifiedFabricIndex;

@property (nonatomic, copy) NSNumber * _Nullable nextCredentialIndex;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRDoorLockClusterClearCredentialParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRDoorLockClusterDlCredential * _Nullable credential;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRWindowCoveringClusterUpOrOpenParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRWindowCoveringClusterDownOrCloseParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRWindowCoveringClusterStopMotionParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRWindowCoveringClusterGoToLiftValueParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull liftValue;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRWindowCoveringClusterGoToLiftPercentageParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull liftPercent100thsValue;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRWindowCoveringClusterGoToTiltValueParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull tiltValue;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRWindowCoveringClusterGoToTiltPercentageParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull tiltPercent100thsValue;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRBarrierControlClusterBarrierControlGoToPercentParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull percentOpen;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRBarrierControlClusterBarrierControlStopParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRThermostatClusterSetpointRaiseLowerParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull mode;

@property (nonatomic, copy) NSNumber * _Nonnull amount;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRThermostatClusterGetWeeklyScheduleResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull numberOfTransitionsForSequence;

@property (nonatomic, copy) NSNumber * _Nonnull dayOfWeekForSequence;

@property (nonatomic, copy) NSNumber * _Nonnull modeForSequence;

@property (nonatomic, copy) NSArray * _Nonnull transitions;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRThermostatClusterSetWeeklyScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull numberOfTransitionsForSequence;

@property (nonatomic, copy) NSNumber * _Nonnull dayOfWeekForSequence;

@property (nonatomic, copy) NSNumber * _Nonnull modeForSequence;

@property (nonatomic, copy) NSArray * _Nonnull transitions;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRThermostatClusterGetWeeklyScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull daysToReturn;

@property (nonatomic, copy) NSNumber * _Nonnull modeToReturn;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRThermostatClusterClearWeeklyScheduleParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRColorControlClusterMoveToHueParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull hue;

@property (nonatomic, copy) NSNumber * _Nonnull direction;

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime;

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask;

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRColorControlClusterMoveHueParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull moveMode;

@property (nonatomic, copy) NSNumber * _Nonnull rate;

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask;

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRColorControlClusterStepHueParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull stepMode;

@property (nonatomic, copy) NSNumber * _Nonnull stepSize;

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime;

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask;

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRColorControlClusterMoveToSaturationParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull saturation;

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime;

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask;

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRColorControlClusterMoveSaturationParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull moveMode;

@property (nonatomic, copy) NSNumber * _Nonnull rate;

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask;

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRColorControlClusterStepSaturationParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull stepMode;

@property (nonatomic, copy) NSNumber * _Nonnull stepSize;

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime;

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask;

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRColorControlClusterMoveToHueAndSaturationParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull hue;

@property (nonatomic, copy) NSNumber * _Nonnull saturation;

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime;

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask;

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRColorControlClusterMoveToColorParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull colorX;

@property (nonatomic, copy) NSNumber * _Nonnull colorY;

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime;

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask;

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRColorControlClusterMoveColorParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull rateX;

@property (nonatomic, copy) NSNumber * _Nonnull rateY;

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask;

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRColorControlClusterStepColorParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull stepX;

@property (nonatomic, copy) NSNumber * _Nonnull stepY;

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime;

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask;

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRColorControlClusterMoveToColorTemperatureParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull colorTemperature;

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime;

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask;

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRColorControlClusterEnhancedMoveToHueParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull enhancedHue;

@property (nonatomic, copy) NSNumber * _Nonnull direction;

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime;

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask;

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRColorControlClusterEnhancedMoveHueParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull moveMode;

@property (nonatomic, copy) NSNumber * _Nonnull rate;

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask;

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRColorControlClusterEnhancedStepHueParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull stepMode;

@property (nonatomic, copy) NSNumber * _Nonnull stepSize;

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime;

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask;

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRColorControlClusterEnhancedMoveToHueAndSaturationParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull enhancedHue;

@property (nonatomic, copy) NSNumber * _Nonnull saturation;

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime;

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask;

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRColorControlClusterColorLoopSetParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull updateFlags;

@property (nonatomic, copy) NSNumber * _Nonnull action;

@property (nonatomic, copy) NSNumber * _Nonnull direction;

@property (nonatomic, copy) NSNumber * _Nonnull time;

@property (nonatomic, copy) NSNumber * _Nonnull startHue;

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask;

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRColorControlClusterStopMoveStepParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask;

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRColorControlClusterMoveColorTemperatureParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull moveMode;

@property (nonatomic, copy) NSNumber * _Nonnull rate;

@property (nonatomic, copy) NSNumber * _Nonnull colorTemperatureMinimumMireds;

@property (nonatomic, copy) NSNumber * _Nonnull colorTemperatureMaximumMireds;

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask;

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRColorControlClusterStepColorTemperatureParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull stepMode;

@property (nonatomic, copy) NSNumber * _Nonnull stepSize;

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime;

@property (nonatomic, copy) NSNumber * _Nonnull colorTemperatureMinimumMireds;

@property (nonatomic, copy) NSNumber * _Nonnull colorTemperatureMaximumMireds;

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask;

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRChannelClusterChangeChannelParams : NSObject <NSCopying>

@property (nonatomic, copy) NSString * _Nonnull match;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRChannelClusterChangeChannelResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status;

@property (nonatomic, copy) NSString * _Nullable data;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRChannelClusterChangeChannelByNumberParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull majorNumber;

@property (nonatomic, copy) NSNumber * _Nonnull minorNumber;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRChannelClusterSkipChannelParams : NSObject <NSCopying>

@property (nonatomic, copy, getter=getCount) NSNumber * _Nonnull count;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTargetNavigatorClusterNavigateTargetParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull target;

@property (nonatomic, copy) NSString * _Nullable data;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTargetNavigatorClusterNavigateTargetResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status;

@property (nonatomic, copy) NSString * _Nullable data;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRMediaPlaybackClusterPlayParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRMediaPlaybackClusterPauseParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRMediaPlaybackClusterStopPlaybackParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRMediaPlaybackClusterStartOverParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRMediaPlaybackClusterPreviousParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRMediaPlaybackClusterNextParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRMediaPlaybackClusterRewindParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRMediaPlaybackClusterFastForwardParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRMediaPlaybackClusterSkipForwardParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull deltaPositionMilliseconds;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRMediaPlaybackClusterSkipBackwardParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull deltaPositionMilliseconds;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRMediaPlaybackClusterPlaybackResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status;

@property (nonatomic, copy) NSString * _Nullable data;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRMediaPlaybackClusterSeekParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull position;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRMediaInputClusterSelectInputParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull index;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRMediaInputClusterShowInputStatusParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRMediaInputClusterHideInputStatusParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRMediaInputClusterRenameInputParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull index;

@property (nonatomic, copy) NSString * _Nonnull name;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRLowPowerClusterSleepParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRKeypadInputClusterSendKeyParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull keyCode;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRKeypadInputClusterSendKeyResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRContentLauncherClusterLaunchContentParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRContentLauncherClusterContentSearch * _Nonnull search;

@property (nonatomic, copy) NSNumber * _Nonnull autoPlay;

@property (nonatomic, copy) NSString * _Nullable data;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRContentLauncherClusterLaunchURLParams : NSObject <NSCopying>

@property (nonatomic, copy) NSString * _Nonnull contentURL;

@property (nonatomic, copy) NSString * _Nullable displayString;

@property (nonatomic, copy) MTRContentLauncherClusterBrandingInformation * _Nullable brandingInformation;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRContentLauncherClusterLaunchResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status;

@property (nonatomic, copy) NSString * _Nullable data;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRAudioOutputClusterSelectOutputParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull index;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRAudioOutputClusterRenameOutputParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull index;

@property (nonatomic, copy) NSString * _Nonnull name;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRApplicationLauncherClusterLaunchAppParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRApplicationLauncherClusterApplication * _Nonnull application;

@property (nonatomic, copy) NSData * _Nullable data;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRApplicationLauncherClusterStopAppParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRApplicationLauncherClusterApplication * _Nonnull application;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRApplicationLauncherClusterHideAppParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRApplicationLauncherClusterApplication * _Nonnull application;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRApplicationLauncherClusterLauncherResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status;

@property (nonatomic, copy) NSData * _Nonnull data;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRAccountLoginClusterGetSetupPINParams : NSObject <NSCopying>

@property (nonatomic, copy) NSString * _Nonnull tempAccountIdentifier;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRAccountLoginClusterGetSetupPINResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSString * _Nonnull setupPIN;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRAccountLoginClusterLoginParams : NSObject <NSCopying>

@property (nonatomic, copy) NSString * _Nonnull tempAccountIdentifier;

@property (nonatomic, copy) NSString * _Nonnull setupPIN;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRAccountLoginClusterLogoutParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRElectricalMeasurementClusterGetProfileInfoResponseCommandParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull profileCount;

@property (nonatomic, copy) NSNumber * _Nonnull profileIntervalPeriod;

@property (nonatomic, copy) NSNumber * _Nonnull maxNumberOfIntervals;

@property (nonatomic, copy) NSArray * _Nonnull listOfAttributes;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRElectricalMeasurementClusterGetProfileInfoCommandParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRElectricalMeasurementClusterGetMeasurementProfileResponseCommandParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull startTime;

@property (nonatomic, copy) NSNumber * _Nonnull status;

@property (nonatomic, copy) NSNumber * _Nonnull profileIntervalPeriod;

@property (nonatomic, copy) NSNumber * _Nonnull numberOfIntervalsDelivered;

@property (nonatomic, copy) NSNumber * _Nonnull attributeId;

@property (nonatomic, copy) NSArray * _Nonnull intervals;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRElectricalMeasurementClusterGetMeasurementProfileCommandParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull attributeId;

@property (nonatomic, copy) NSNumber * _Nonnull startTime;

@property (nonatomic, copy) NSNumber * _Nonnull numberOfIntervals;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTestParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTestSpecificResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull returnValue;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTestNotHandledParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTestAddArgumentsResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull returnValue;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTestSpecificParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTestSimpleArgumentResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull returnValue;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTestUnknownCommandParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTestStructArrayArgumentResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull arg1;

@property (nonatomic, copy) NSArray * _Nonnull arg2;

@property (nonatomic, copy) NSArray * _Nonnull arg3;

@property (nonatomic, copy) NSArray * _Nonnull arg4;

@property (nonatomic, copy) NSNumber * _Nonnull arg5;

@property (nonatomic, copy) NSNumber * _Nonnull arg6;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTestAddArgumentsParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull arg1;

@property (nonatomic, copy) NSNumber * _Nonnull arg2;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTestListInt8UReverseResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull arg1;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTestSimpleArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull arg1;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTestEnumsResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull arg1;

@property (nonatomic, copy) NSNumber * _Nonnull arg2;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTestStructArrayArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull arg1;

@property (nonatomic, copy) NSArray * _Nonnull arg2;

@property (nonatomic, copy) NSArray * _Nonnull arg3;

@property (nonatomic, copy) NSArray * _Nonnull arg4;

@property (nonatomic, copy) NSNumber * _Nonnull arg5;

@property (nonatomic, copy) NSNumber * _Nonnull arg6;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTestNullableOptionalResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull wasPresent;

@property (nonatomic, copy) NSNumber * _Nullable wasNull;

@property (nonatomic, copy) NSNumber * _Nullable value;

@property (nonatomic, copy) NSNumber * _Nullable originalValue;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTestStructArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRTestClusterClusterSimpleStruct * _Nonnull arg1;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTestComplexNullableOptionalResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull nullableIntWasNull;

@property (nonatomic, copy) NSNumber * _Nullable nullableIntValue;

@property (nonatomic, copy) NSNumber * _Nonnull optionalIntWasPresent;

@property (nonatomic, copy) NSNumber * _Nullable optionalIntValue;

@property (nonatomic, copy) NSNumber * _Nonnull nullableOptionalIntWasPresent;

@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalIntWasNull;

@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalIntValue;

@property (nonatomic, copy) NSNumber * _Nonnull nullableStringWasNull;

@property (nonatomic, copy) NSString * _Nullable nullableStringValue;

@property (nonatomic, copy) NSNumber * _Nonnull optionalStringWasPresent;

@property (nonatomic, copy) NSString * _Nullable optionalStringValue;

@property (nonatomic, copy) NSNumber * _Nonnull nullableOptionalStringWasPresent;

@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalStringWasNull;

@property (nonatomic, copy) NSString * _Nullable nullableOptionalStringValue;

@property (nonatomic, copy) NSNumber * _Nonnull nullableStructWasNull;

@property (nonatomic, copy) MTRTestClusterClusterSimpleStruct * _Nullable nullableStructValue;

@property (nonatomic, copy) NSNumber * _Nonnull optionalStructWasPresent;

@property (nonatomic, copy) MTRTestClusterClusterSimpleStruct * _Nullable optionalStructValue;

@property (nonatomic, copy) NSNumber * _Nonnull nullableOptionalStructWasPresent;

@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalStructWasNull;

@property (nonatomic, copy) MTRTestClusterClusterSimpleStruct * _Nullable nullableOptionalStructValue;

@property (nonatomic, copy) NSNumber * _Nonnull nullableListWasNull;

@property (nonatomic, copy) NSArray * _Nullable nullableListValue;

@property (nonatomic, copy) NSNumber * _Nonnull optionalListWasPresent;

@property (nonatomic, copy) NSArray * _Nullable optionalListValue;

@property (nonatomic, copy) NSNumber * _Nonnull nullableOptionalListWasPresent;

@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalListWasNull;

@property (nonatomic, copy) NSArray * _Nullable nullableOptionalListValue;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTestNestedStructArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRTestClusterClusterNestedStruct * _Nonnull arg1;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterBooleanResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull value;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTestListStructArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull arg1;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterSimpleStructResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRTestClusterClusterSimpleStruct * _Nonnull arg1;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTestListInt8UArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull arg1;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTestEmitTestEventResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull value;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTestNestedStructListArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRTestClusterClusterNestedStructList * _Nonnull arg1;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTestEmitTestFabricScopedEventResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull value;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTestListNestedStructListArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull arg1;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTestListInt8UReverseRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull arg1;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTestEnumsRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull arg1;

@property (nonatomic, copy) NSNumber * _Nonnull arg2;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTestNullableOptionalRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nullable arg1;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTestComplexNullableOptionalRequestParams : NSObject <NSCopying>

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
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterSimpleStructEchoRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRTestClusterClusterSimpleStruct * _Nonnull arg1;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTimedInvokeRequestParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTestSimpleOptionalArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nullable arg1;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTestEmitTestEventRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull arg1;

@property (nonatomic, copy) NSNumber * _Nonnull arg2;

@property (nonatomic, copy) NSNumber * _Nonnull arg3;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRTestClusterClusterTestEmitTestFabricScopedEventRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull arg1;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRFaultInjectionClusterFailAtFaultParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull type;

@property (nonatomic, copy) NSNumber * _Nonnull id;

@property (nonatomic, copy) NSNumber * _Nonnull numCallsToSkip;

@property (nonatomic, copy) NSNumber * _Nonnull numCallsToFail;

@property (nonatomic, copy) NSNumber * _Nonnull takeMutex;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end
@interface MTRFaultInjectionClusterFailRandomlyAtFaultParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull type;

@property (nonatomic, copy) NSNumber * _Nonnull id;

@property (nonatomic, copy) NSNumber * _Nonnull percentage;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

- (instancetype)init;
- (id)copyWithZone:(NSZone * _Nullable)zone;
@end

NS_ASSUME_NONNULL_END
