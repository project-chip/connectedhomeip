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

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRIdentifyClusterIdentifyParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull identifyTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRIdentifyClusterTriggerEffectParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull effectIdentifier API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull effectVariant API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupsClusterAddGroupParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSString * _Nonnull groupName API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

@interface MTRGroupsClusterAddGroupParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    MTR_NEWLY_DEPRECATED("Please use groupID");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupsClusterAddGroupResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_NEWLY_AVAILABLE;
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

@interface MTRGroupsClusterAddGroupResponseParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    MTR_NEWLY_DEPRECATED("Please use groupID");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupsClusterViewGroupParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_NEWLY_AVAILABLE;
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
@end

@interface MTRGroupsClusterViewGroupParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    MTR_NEWLY_DEPRECATED("Please use groupID");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupsClusterViewGroupResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSString * _Nonnull groupName API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

@interface MTRGroupsClusterViewGroupResponseParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    MTR_NEWLY_DEPRECATED("Please use groupID");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupsClusterGetGroupMembershipParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull groupList API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupsClusterGetGroupMembershipResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nullable capacity API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSArray * _Nonnull groupList API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupsClusterRemoveGroupParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_NEWLY_AVAILABLE;
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
@end

@interface MTRGroupsClusterRemoveGroupParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    MTR_NEWLY_DEPRECATED("Please use groupID");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupsClusterRemoveGroupResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_NEWLY_AVAILABLE;
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

@interface MTRGroupsClusterRemoveGroupResponseParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    MTR_NEWLY_DEPRECATED("Please use groupID");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupsClusterAddGroupIfIdentifyingParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSString * _Nonnull groupName API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

@interface MTRGroupsClusterAddGroupIfIdentifyingParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    MTR_NEWLY_DEPRECATED("Please use groupID");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterAddSceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nonnull sceneName API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSArray * _Nonnull extensionFieldSets API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterAddSceneResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterViewSceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterViewSceneResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable transitionTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable sceneName API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSArray * _Nullable extensionFieldSets API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterRemoveSceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterRemoveSceneResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterRemoveAllScenesParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterRemoveAllScenesResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterStoreSceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterStoreSceneResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterRecallSceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable transitionTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterGetSceneMembershipParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterGetSceneMembershipResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable capacity API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSArray * _Nullable sceneList API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterEnhancedAddSceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nonnull sceneName API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSArray * _Nonnull extensionFieldSets API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterEnhancedAddSceneResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterEnhancedViewSceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterEnhancedViewSceneResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable transitionTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable sceneName API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSArray * _Nullable extensionFieldSets API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterCopySceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull mode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupIdFrom API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull sceneIdFrom API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupIdTo API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull sceneIdTo API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterCopySceneResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupIdFrom API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull sceneIdFrom API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROnOffClusterOffWithEffectParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull effectIdentifier MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull effectVariant API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

@interface MTROnOffClusterOffWithEffectParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull effectId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    MTR_NEWLY_DEPRECATED("Please use effectIdentifier");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROnOffClusterOnWithTimedOffParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull onOffControl API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull onTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull offWaitTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRLevelControlClusterMoveToLevelParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull level API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable transitionTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRLevelControlClusterMoveParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull moveMode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable rate API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRLevelControlClusterStepParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull stepMode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull stepSize API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable transitionTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRLevelControlClusterStopParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRLevelControlClusterMoveToLevelWithOnOffParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull level API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable transitionTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRLevelControlClusterMoveWithOnOffParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull moveMode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable rate API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRLevelControlClusterStepWithOnOffParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull stepMode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull stepSize API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable transitionTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRLevelControlClusterStopWithOnOffParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRLevelControlClusterMoveToClosestFrequencyParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull frequency API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterInstantActionParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable invokeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterInstantActionWithTransitionParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable invokeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterStartActionParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable invokeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterStartActionWithDurationParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable invokeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull duration API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterStopActionParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable invokeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterPauseActionParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable invokeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterPauseActionWithDurationParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable invokeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull duration API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterResumeActionParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable invokeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterEnableActionParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable invokeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterEnableActionWithDurationParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable invokeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull duration API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterDisableActionParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable invokeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterDisableActionWithDurationParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable invokeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull duration API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("This command has been removed")
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
@end

MTR_NEWLY_AVAILABLE
@interface MTROTASoftwareUpdateProviderClusterQueryImageParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull vendorID MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull productID MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSArray * _Nonnull protocolsSupported MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nullable hardwareVersion MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSString * _Nullable location MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nullable requestorCanConsent MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSData * _Nullable metadataForProvider MTR_NEWLY_AVAILABLE;
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateProviderClusterQueryImageParams")
@interface MTROtaSoftwareUpdateProviderClusterQueryImageParams : MTROTASoftwareUpdateProviderClusterQueryImageParams

@property (nonatomic, copy) NSNumber * _Nonnull vendorId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    MTR_NEWLY_DEPRECATED("Please use vendorID");

@property (nonatomic, copy) NSNumber * _Nonnull productId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    MTR_NEWLY_DEPRECATED("Please use productID");
@end

MTR_NEWLY_AVAILABLE
@interface MTROTASoftwareUpdateProviderClusterQueryImageResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nullable delayedActionTime MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSString * _Nullable imageURI MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nullable softwareVersion MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSString * _Nullable softwareVersionString MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSData * _Nullable updateToken MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nullable userConsentNeeded MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSData * _Nullable metadataForRequestor MTR_NEWLY_AVAILABLE;
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateProviderClusterQueryImageResponseParams")
@interface MTROtaSoftwareUpdateProviderClusterQueryImageResponseParams : MTROTASoftwareUpdateProviderClusterQueryImageResponseParams
@end

MTR_NEWLY_AVAILABLE
@interface MTROTASoftwareUpdateProviderClusterApplyUpdateRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull updateToken MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy, getter=getNewVersion) NSNumber * _Nonnull newVersion MTR_NEWLY_AVAILABLE;
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateProviderClusterApplyUpdateRequestParams")
@interface MTROtaSoftwareUpdateProviderClusterApplyUpdateRequestParams : MTROTASoftwareUpdateProviderClusterApplyUpdateRequestParams
@end

MTR_NEWLY_AVAILABLE
@interface MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull action MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull delayedActionTime MTR_NEWLY_AVAILABLE;
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams")
@interface MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams
    : MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams
@end

MTR_NEWLY_AVAILABLE
@interface MTROTASoftwareUpdateProviderClusterNotifyUpdateAppliedParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull updateToken MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion MTR_NEWLY_AVAILABLE;
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateProviderClusterNotifyUpdateAppliedParams")
@interface MTROtaSoftwareUpdateProviderClusterNotifyUpdateAppliedParams
    : MTROTASoftwareUpdateProviderClusterNotifyUpdateAppliedParams
@end

MTR_NEWLY_AVAILABLE
@interface MTROTASoftwareUpdateRequestorClusterAnnounceOTAProviderParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull providerNodeID MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull vendorID MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull announcementReason MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSData * _Nullable metadataForNode MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull endpoint MTR_NEWLY_AVAILABLE;
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterAnnounceOTAProviderParams")
@interface MTROtaSoftwareUpdateRequestorClusterAnnounceOtaProviderParams
    : MTROTASoftwareUpdateRequestorClusterAnnounceOTAProviderParams

@property (nonatomic, copy) NSNumber * _Nonnull providerNodeId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    MTR_NEWLY_DEPRECATED("Please use providerNodeID");

@property (nonatomic, copy) NSNumber * _Nonnull vendorId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    MTR_NEWLY_DEPRECATED("Please use vendorID");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGeneralCommissioningClusterArmFailSafeParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull expiryLengthSeconds API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull breadcrumb API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGeneralCommissioningClusterArmFailSafeResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull errorCode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nonnull debugText API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGeneralCommissioningClusterSetRegulatoryConfigParams : NSObject <NSCopying>

@property (nonatomic, copy, getter=getNewRegulatoryConfig)
    NSNumber * _Nonnull newRegulatoryConfig API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nonnull countryCode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull breadcrumb API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGeneralCommissioningClusterSetRegulatoryConfigResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull errorCode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nonnull debugText API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGeneralCommissioningClusterCommissioningCompleteResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull errorCode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nonnull debugText API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRNetworkCommissioningClusterScanNetworksParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nullable ssid API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable breadcrumb API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRNetworkCommissioningClusterScanNetworksResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull networkingStatus API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable debugText API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSArray * _Nullable wiFiScanResults API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSArray * _Nullable threadScanResults API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRNetworkCommissioningClusterAddOrUpdateWiFiNetworkParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull ssid API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSData * _Nonnull credentials API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable breadcrumb API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRNetworkCommissioningClusterAddOrUpdateThreadNetworkParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull operationalDataset API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable breadcrumb API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRNetworkCommissioningClusterRemoveNetworkParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull networkID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable breadcrumb API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRNetworkCommissioningClusterNetworkConfigResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull networkingStatus API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable debugText API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable networkIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRNetworkCommissioningClusterConnectNetworkParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull networkID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable breadcrumb API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRNetworkCommissioningClusterConnectNetworkResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull networkingStatus API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable debugText API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable errorValue API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRNetworkCommissioningClusterReorderNetworkParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull networkID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull networkIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable breadcrumb API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDiagnosticLogsClusterRetrieveLogsRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull intent API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull requestedProtocol API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSData * _Nonnull transferFileDesignator API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    ;
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDiagnosticLogsClusterRetrieveLogsResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSData * _Nonnull logContent MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull utcTimeStamp MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull timeSinceBoot API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

@interface MTRDiagnosticLogsClusterRetrieveLogsResponseParams (Deprecated)

@property (nonatomic, copy) NSData * _Nonnull content API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    MTR_NEWLY_DEPRECATED("Please use logContent");

@property (nonatomic, copy) NSNumber * _Nonnull timeStamp API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    MTR_NEWLY_DEPRECATED("Please use utcTimeStamp");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGeneralDiagnosticsClusterTestEventTriggerParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull enableKey API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull eventTrigger API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRTimeSynchronizationClusterSetUtcTimeParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull utcTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull granularity API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable timeSource API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRAdministratorCommissioningClusterOpenCommissioningWindowParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull commissioningTimeout API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    ;

@property (nonatomic, copy) NSData * _Nonnull pakePasscodeVerifier MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull discriminator API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull iterations API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSData * _Nonnull salt API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

@interface MTRAdministratorCommissioningClusterOpenCommissioningWindowParams (Deprecated)

@property (nonatomic, copy) NSData * _Nonnull pakeVerifier API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    MTR_NEWLY_DEPRECATED("Please use pakePasscodeVerifier");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRAdministratorCommissioningClusterOpenBasicCommissioningWindowParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull commissioningTimeout API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    ;
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROperationalCredentialsClusterAttestationRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull attestationNonce API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROperationalCredentialsClusterAttestationResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull attestationElements API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSData * _Nonnull attestationSignature MTR_NEWLY_AVAILABLE;
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

@interface MTROperationalCredentialsClusterAttestationResponseParams (Deprecated)

@property (nonatomic, copy) NSData * _Nonnull signature API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    MTR_NEWLY_DEPRECATED("Please use attestationSignature");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROperationalCredentialsClusterCertificateChainRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull certificateType API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROperationalCredentialsClusterCertificateChainResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull certificate API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROperationalCredentialsClusterCSRRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull csrNonce API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable isForUpdateNOC API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROperationalCredentialsClusterCSRResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull nocsrElements API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSData * _Nonnull attestationSignature API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROperationalCredentialsClusterAddNOCParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull nocValue API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSData * _Nullable icacValue API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSData * _Nonnull ipkValue API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull caseAdminSubject API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull adminVendorId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROperationalCredentialsClusterUpdateNOCParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull nocValue API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSData * _Nullable icacValue API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROperationalCredentialsClusterNOCResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull statusCode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable fabricIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable debugText API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROperationalCredentialsClusterUpdateFabricLabelParams : NSObject <NSCopying>

@property (nonatomic, copy) NSString * _Nonnull label API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROperationalCredentialsClusterRemoveFabricParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROperationalCredentialsClusterAddTrustedRootCertificateParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull rootCACertificate MTR_NEWLY_AVAILABLE;
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
@end

@interface MTROperationalCredentialsClusterAddTrustedRootCertificateParams (Deprecated)

@property (nonatomic, copy) NSData * _Nonnull rootCertificate API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    MTR_NEWLY_DEPRECATED("Please use rootCACertificate");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupKeyManagementClusterKeySetWriteParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRGroupKeyManagementClusterGroupKeySetStruct * _Nonnull groupKeySet API_AVAILABLE(
    ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupKeyManagementClusterKeySetReadParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupKeySetID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupKeyManagementClusterKeySetReadResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRGroupKeyManagementClusterGroupKeySetStruct * _Nonnull groupKeySet API_AVAILABLE(
    ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupKeyManagementClusterKeySetRemoveParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupKeySetID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupKeyManagementClusterKeySetReadAllIndicesParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull groupKeySetIDs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupKeyManagementClusterKeySetReadAllIndicesResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull groupKeySetIDs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRModeSelectClusterChangeToModeParams : NSObject <NSCopying>

@property (nonatomic, copy, getter=getNewMode)
    NSNumber * _Nonnull newMode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterLockDoorParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nullable pinCode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterUnlockDoorParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nullable pinCode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterUnlockWithTimeoutParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull timeout API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSData * _Nullable pinCode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterSetWeekDayScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull weekDayIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull userIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull daysMask API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull startHour API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull startMinute API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull endHour API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull endMinute API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterGetWeekDayScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull weekDayIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull userIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterGetWeekDayScheduleResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull weekDayIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull userIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable daysMask API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable startHour API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable startMinute API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable endHour API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable endMinute API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterClearWeekDayScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull weekDayIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull userIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterSetYearDayScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull yearDayIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull userIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull localStartTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull localEndTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterGetYearDayScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull yearDayIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull userIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterGetYearDayScheduleResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull yearDayIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull userIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable localStartTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable localEndTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterClearYearDayScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull yearDayIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull userIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterSetHolidayScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull holidayIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull localStartTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull localEndTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull operatingMode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterGetHolidayScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull holidayIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterGetHolidayScheduleResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull holidayIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable localStartTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable localEndTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable operatingMode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterClearHolidayScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull holidayIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterSetUserParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull operationType API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull userIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable userName API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable userUniqueID MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nullable userStatus API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable userType API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable credentialRule API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

@interface MTRDoorLockClusterSetUserParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nullable userUniqueId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    MTR_NEWLY_DEPRECATED("Please use userUniqueID");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterGetUserParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull userIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterGetUserResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull userIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable userName API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable userUniqueID MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nullable userStatus API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable userType API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable credentialRule API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSArray * _Nullable credentials API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable creatorFabricIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy)
    NSNumber * _Nullable lastModifiedFabricIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable nextUserIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

@interface MTRDoorLockClusterGetUserResponseParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nullable userUniqueId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    MTR_NEWLY_DEPRECATED("Please use userUniqueID");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterClearUserParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull userIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterSetCredentialParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull operationType API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy)
    MTRDoorLockClusterCredentialStruct * _Nonnull credential API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSData * _Nonnull credentialData API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable userIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable userStatus API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable userType API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterSetCredentialResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable userIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable nextCredentialIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    ;
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterGetCredentialStatusParams : NSObject <NSCopying>

@property (nonatomic, copy)
    MTRDoorLockClusterCredentialStruct * _Nonnull credential API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterGetCredentialStatusResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull credentialExists API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable userIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable creatorFabricIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy)
    NSNumber * _Nullable lastModifiedFabricIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable nextCredentialIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    ;
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterClearCredentialParams : NSObject <NSCopying>

@property (nonatomic, copy)
    MTRDoorLockClusterCredentialStruct * _Nullable credential API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRWindowCoveringClusterGoToLiftValueParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull liftValue API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRWindowCoveringClusterGoToLiftPercentageParams : NSObject <NSCopying>

@property (nonatomic, copy)
    NSNumber * _Nonnull liftPercent100thsValue API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRWindowCoveringClusterGoToTiltValueParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull tiltValue API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRWindowCoveringClusterGoToTiltPercentageParams : NSObject <NSCopying>

@property (nonatomic, copy)
    NSNumber * _Nonnull tiltPercent100thsValue API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRBarrierControlClusterBarrierControlGoToPercentParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull percentOpen API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRThermostatClusterSetpointRaiseLowerParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull mode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull amount API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRThermostatClusterGetWeeklyScheduleResponseParams : NSObject <NSCopying>

@property (nonatomic, copy)
    NSNumber * _Nonnull numberOfTransitionsForSequence API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull dayOfWeekForSequence API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    ;

@property (nonatomic, copy) NSNumber * _Nonnull modeForSequence API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSArray * _Nonnull transitions API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRThermostatClusterSetWeeklyScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy)
    NSNumber * _Nonnull numberOfTransitionsForSequence API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull dayOfWeekForSequence API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    ;

@property (nonatomic, copy) NSNumber * _Nonnull modeForSequence API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSArray * _Nonnull transitions API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRThermostatClusterGetWeeklyScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull daysToReturn API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull modeToReturn API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterMoveToHueParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull hue API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull direction API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterMoveHueParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull moveMode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull rate API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterStepHueParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull stepMode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull stepSize API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterMoveToSaturationParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull saturation API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterMoveSaturationParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull moveMode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull rate API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterStepSaturationParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull stepMode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull stepSize API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterMoveToHueAndSaturationParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull hue API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull saturation API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterMoveToColorParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull colorX API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull colorY API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterMoveColorParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull rateX API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull rateY API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterStepColorParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull stepX API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull stepY API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterMoveToColorTemperatureParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull colorTemperatureMireds MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

@interface MTRColorControlClusterMoveToColorTemperatureParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull colorTemperature API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    MTR_NEWLY_DEPRECATED("Please use colorTemperatureMireds");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterEnhancedMoveToHueParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull enhancedHue API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull direction API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterEnhancedMoveHueParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull moveMode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull rate API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterEnhancedStepHueParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull stepMode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull stepSize API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterEnhancedMoveToHueAndSaturationParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull enhancedHue API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull saturation API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterColorLoopSetParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull updateFlags API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull action API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull direction API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull time API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull startHue API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterStopMoveStepParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterMoveColorTemperatureParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull moveMode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull rate API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy)
    NSNumber * _Nonnull colorTemperatureMinimumMireds API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy)
    NSNumber * _Nonnull colorTemperatureMaximumMireds API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterStepColorTemperatureParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull stepMode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull stepSize API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy)
    NSNumber * _Nonnull colorTemperatureMinimumMireds API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy)
    NSNumber * _Nonnull colorTemperatureMaximumMireds API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRChannelClusterChangeChannelParams : NSObject <NSCopying>

@property (nonatomic, copy) NSString * _Nonnull match API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRChannelClusterChangeChannelResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable data API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRChannelClusterChangeChannelByNumberParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull majorNumber API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull minorNumber API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRChannelClusterSkipChannelParams : NSObject <NSCopying>

@property (nonatomic, copy, getter=getCount)
    NSNumber * _Nonnull count API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRTargetNavigatorClusterNavigateTargetParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull target API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable data API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRTargetNavigatorClusterNavigateTargetResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable data API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRMediaPlaybackClusterSkipForwardParams : NSObject <NSCopying>

@property (nonatomic, copy)
    NSNumber * _Nonnull deltaPositionMilliseconds API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRMediaPlaybackClusterSkipBackwardParams : NSObject <NSCopying>

@property (nonatomic, copy)
    NSNumber * _Nonnull deltaPositionMilliseconds API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRMediaPlaybackClusterPlaybackResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable data API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRMediaPlaybackClusterSeekParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull position API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRMediaInputClusterSelectInputParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull index API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRMediaInputClusterRenameInputParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull index API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nonnull name API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRKeypadInputClusterSendKeyParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull keyCode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRKeypadInputClusterSendKeyResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRContentLauncherClusterLaunchContentParams : NSObject <NSCopying>

@property (nonatomic, copy)
    MTRContentLauncherClusterContentSearch * _Nonnull search API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull autoPlay API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable data API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRContentLauncherClusterLaunchURLParams : NSObject <NSCopying>

@property (nonatomic, copy) NSString * _Nonnull contentURL API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable displayString API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) MTRContentLauncherClusterBrandingInformation * _Nullable brandingInformation API_AVAILABLE(
    ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRContentLauncherClusterLaunchResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable data API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRAudioOutputClusterSelectOutputParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull index API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRAudioOutputClusterRenameOutputParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull index API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nonnull name API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRApplicationLauncherClusterLaunchAppParams : NSObject <NSCopying>

@property (nonatomic, copy)
    MTRApplicationLauncherClusterApplication * _Nonnull application API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSData * _Nullable data API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRApplicationLauncherClusterStopAppParams : NSObject <NSCopying>

@property (nonatomic, copy)
    MTRApplicationLauncherClusterApplication * _Nonnull application API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRApplicationLauncherClusterHideAppParams : NSObject <NSCopying>

@property (nonatomic, copy)
    MTRApplicationLauncherClusterApplication * _Nonnull application API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRApplicationLauncherClusterLauncherResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSData * _Nonnull data API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRAccountLoginClusterGetSetupPINParams : NSObject <NSCopying>

@property (nonatomic, copy)
    NSString * _Nonnull tempAccountIdentifier API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRAccountLoginClusterGetSetupPINResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSString * _Nonnull setupPIN API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRAccountLoginClusterLoginParams : NSObject <NSCopying>

@property (nonatomic, copy)
    NSString * _Nonnull tempAccountIdentifier API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nonnull setupPIN API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRElectricalMeasurementClusterGetProfileInfoResponseCommandParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull profileCount API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy)
    NSNumber * _Nonnull profileIntervalPeriod API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull maxNumberOfIntervals API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    ;

@property (nonatomic, copy) NSArray * _Nonnull listOfAttributes API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRElectricalMeasurementClusterGetMeasurementProfileResponseCommandParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull startTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy)
    NSNumber * _Nonnull profileIntervalPeriod API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy)
    NSNumber * _Nonnull numberOfIntervalsDelivered API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull attributeId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSArray * _Nonnull intervals API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRElectricalMeasurementClusterGetMeasurementProfileCommandParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull attributeId API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull startTime API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull numberOfIntervals API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestParams : NSObject <NSCopying>
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestParams")
@interface MTRTestClusterClusterTestParams : MTRUnitTestingClusterTestParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestSpecificResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull returnValue MTR_NEWLY_AVAILABLE;
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestSpecificResponseParams")
@interface MTRTestClusterClusterTestSpecificResponseParams : MTRUnitTestingClusterTestSpecificResponseParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestNotHandledParams : NSObject <NSCopying>
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestNotHandledParams")
@interface MTRTestClusterClusterTestNotHandledParams : MTRUnitTestingClusterTestNotHandledParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestAddArgumentsResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull returnValue MTR_NEWLY_AVAILABLE;
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestAddArgumentsResponseParams")
@interface MTRTestClusterClusterTestAddArgumentsResponseParams : MTRUnitTestingClusterTestAddArgumentsResponseParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestSpecificParams : NSObject <NSCopying>
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestSpecificParams")
@interface MTRTestClusterClusterTestSpecificParams : MTRUnitTestingClusterTestSpecificParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestSimpleArgumentResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull returnValue MTR_NEWLY_AVAILABLE;
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestSimpleArgumentResponseParams")
@interface MTRTestClusterClusterTestSimpleArgumentResponseParams : MTRUnitTestingClusterTestSimpleArgumentResponseParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestUnknownCommandParams : NSObject <NSCopying>
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestUnknownCommandParams")
@interface MTRTestClusterClusterTestUnknownCommandParams : MTRUnitTestingClusterTestUnknownCommandParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestStructArrayArgumentResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull arg1 MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSArray * _Nonnull arg2 MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSArray * _Nonnull arg3 MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSArray * _Nonnull arg4 MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull arg5 MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull arg6 MTR_NEWLY_AVAILABLE;
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestStructArrayArgumentResponseParams")
@interface MTRTestClusterClusterTestStructArrayArgumentResponseParams : MTRUnitTestingClusterTestStructArrayArgumentResponseParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestAddArgumentsParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull arg1 MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull arg2 MTR_NEWLY_AVAILABLE;
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestAddArgumentsParams")
@interface MTRTestClusterClusterTestAddArgumentsParams : MTRUnitTestingClusterTestAddArgumentsParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestListInt8UReverseResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull arg1 MTR_NEWLY_AVAILABLE;
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestListInt8UReverseResponseParams")
@interface MTRTestClusterClusterTestListInt8UReverseResponseParams : MTRUnitTestingClusterTestListInt8UReverseResponseParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestSimpleArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull arg1 MTR_NEWLY_AVAILABLE;
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestSimpleArgumentRequestParams")
@interface MTRTestClusterClusterTestSimpleArgumentRequestParams : MTRUnitTestingClusterTestSimpleArgumentRequestParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestEnumsResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull arg1 MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull arg2 MTR_NEWLY_AVAILABLE;
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestEnumsResponseParams")
@interface MTRTestClusterClusterTestEnumsResponseParams : MTRUnitTestingClusterTestEnumsResponseParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestStructArrayArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull arg1 MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSArray * _Nonnull arg2 MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSArray * _Nonnull arg3 MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSArray * _Nonnull arg4 MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull arg5 MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull arg6 MTR_NEWLY_AVAILABLE;
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestStructArrayArgumentRequestParams")
@interface MTRTestClusterClusterTestStructArrayArgumentRequestParams : MTRUnitTestingClusterTestStructArrayArgumentRequestParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestNullableOptionalResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull wasPresent MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nullable wasNull MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nullable value MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nullable originalValue MTR_NEWLY_AVAILABLE;
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestNullableOptionalResponseParams")
@interface MTRTestClusterClusterTestNullableOptionalResponseParams : MTRUnitTestingClusterTestNullableOptionalResponseParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestStructArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nonnull arg1 MTR_NEWLY_AVAILABLE;
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestStructArgumentRequestParams")
@interface MTRTestClusterClusterTestStructArgumentRequestParams : MTRUnitTestingClusterTestStructArgumentRequestParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestComplexNullableOptionalResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull nullableIntWasNull MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nullable nullableIntValue MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull optionalIntWasPresent MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nullable optionalIntValue MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull nullableOptionalIntWasPresent MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalIntWasNull MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalIntValue MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull nullableStringWasNull MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSString * _Nullable nullableStringValue MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull optionalStringWasPresent MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSString * _Nullable optionalStringValue MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull nullableOptionalStringWasPresent MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalStringWasNull MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSString * _Nullable nullableOptionalStringValue MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull nullableStructWasNull MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable nullableStructValue MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull optionalStructWasPresent MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable optionalStructValue MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull nullableOptionalStructWasPresent MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalStructWasNull MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable nullableOptionalStructValue MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull nullableListWasNull MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSArray * _Nullable nullableListValue MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull optionalListWasPresent MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSArray * _Nullable optionalListValue MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull nullableOptionalListWasPresent MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalListWasNull MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSArray * _Nullable nullableOptionalListValue MTR_NEWLY_AVAILABLE;
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestComplexNullableOptionalResponseParams")
@interface MTRTestClusterClusterTestComplexNullableOptionalResponseParams
    : MTRUnitTestingClusterTestComplexNullableOptionalResponseParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestNestedStructArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRUnitTestingClusterNestedStruct * _Nonnull arg1 MTR_NEWLY_AVAILABLE;
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestNestedStructArgumentRequestParams")
@interface MTRTestClusterClusterTestNestedStructArgumentRequestParams : MTRUnitTestingClusterTestNestedStructArgumentRequestParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterBooleanResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull value MTR_NEWLY_AVAILABLE;
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterBooleanResponseParams")
@interface MTRTestClusterClusterBooleanResponseParams : MTRUnitTestingClusterBooleanResponseParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestListStructArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull arg1 MTR_NEWLY_AVAILABLE;
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestListStructArgumentRequestParams")
@interface MTRTestClusterClusterTestListStructArgumentRequestParams : MTRUnitTestingClusterTestListStructArgumentRequestParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterSimpleStructResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nonnull arg1 MTR_NEWLY_AVAILABLE;
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterSimpleStructResponseParams")
@interface MTRTestClusterClusterSimpleStructResponseParams : MTRUnitTestingClusterSimpleStructResponseParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestListInt8UArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull arg1 MTR_NEWLY_AVAILABLE;
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestListInt8UArgumentRequestParams")
@interface MTRTestClusterClusterTestListInt8UArgumentRequestParams : MTRUnitTestingClusterTestListInt8UArgumentRequestParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestEmitTestEventResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull value MTR_NEWLY_AVAILABLE;
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestEmitTestEventResponseParams")
@interface MTRTestClusterClusterTestEmitTestEventResponseParams : MTRUnitTestingClusterTestEmitTestEventResponseParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestNestedStructListArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRUnitTestingClusterNestedStructList * _Nonnull arg1 MTR_NEWLY_AVAILABLE;
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestNestedStructListArgumentRequestParams")
@interface MTRTestClusterClusterTestNestedStructListArgumentRequestParams
    : MTRUnitTestingClusterTestNestedStructListArgumentRequestParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestEmitTestFabricScopedEventResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull value MTR_NEWLY_AVAILABLE;
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
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
    tvos(16.1)) MTR_NEWLY_DEPRECATED("Timed invoke does not make sense for server to client commands");
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestEmitTestFabricScopedEventResponseParams")
@interface MTRTestClusterClusterTestEmitTestFabricScopedEventResponseParams
    : MTRUnitTestingClusterTestEmitTestFabricScopedEventResponseParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestListNestedStructListArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull arg1 MTR_NEWLY_AVAILABLE;
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestListNestedStructListArgumentRequestParams")
@interface MTRTestClusterClusterTestListNestedStructListArgumentRequestParams
    : MTRUnitTestingClusterTestListNestedStructListArgumentRequestParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestListInt8UReverseRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull arg1 MTR_NEWLY_AVAILABLE;
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestListInt8UReverseRequestParams")
@interface MTRTestClusterClusterTestListInt8UReverseRequestParams : MTRUnitTestingClusterTestListInt8UReverseRequestParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestEnumsRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull arg1 MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull arg2 MTR_NEWLY_AVAILABLE;
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestEnumsRequestParams")
@interface MTRTestClusterClusterTestEnumsRequestParams : MTRUnitTestingClusterTestEnumsRequestParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestNullableOptionalRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nullable arg1 MTR_NEWLY_AVAILABLE;
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestNullableOptionalRequestParams")
@interface MTRTestClusterClusterTestNullableOptionalRequestParams : MTRUnitTestingClusterTestNullableOptionalRequestParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestComplexNullableOptionalRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nullable nullableInt MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nullable optionalInt MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalInt MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSString * _Nullable nullableString MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSString * _Nullable optionalString MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSString * _Nullable nullableOptionalString MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable nullableStruct MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable optionalStruct MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable nullableOptionalStruct MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSArray * _Nullable nullableList MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSArray * _Nullable optionalList MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSArray * _Nullable nullableOptionalList MTR_NEWLY_AVAILABLE;
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestComplexNullableOptionalRequestParams")
@interface MTRTestClusterClusterTestComplexNullableOptionalRequestParams
    : MTRUnitTestingClusterTestComplexNullableOptionalRequestParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterSimpleStructEchoRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nonnull arg1 MTR_NEWLY_AVAILABLE;
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterSimpleStructEchoRequestParams")
@interface MTRTestClusterClusterSimpleStructEchoRequestParams : MTRUnitTestingClusterSimpleStructEchoRequestParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTimedInvokeRequestParams : NSObject <NSCopying>
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTimedInvokeRequestParams")
@interface MTRTestClusterClusterTimedInvokeRequestParams : MTRUnitTestingClusterTimedInvokeRequestParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestSimpleOptionalArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nullable arg1 MTR_NEWLY_AVAILABLE;
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestSimpleOptionalArgumentRequestParams")
@interface MTRTestClusterClusterTestSimpleOptionalArgumentRequestParams
    : MTRUnitTestingClusterTestSimpleOptionalArgumentRequestParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestEmitTestEventRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull arg1 MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull arg2 MTR_NEWLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull arg3 MTR_NEWLY_AVAILABLE;
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestEmitTestEventRequestParams")
@interface MTRTestClusterClusterTestEmitTestEventRequestParams : MTRUnitTestingClusterTestEmitTestEventRequestParams
@end

MTR_NEWLY_AVAILABLE
@interface MTRUnitTestingClusterTestEmitTestFabricScopedEventRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull arg1 MTR_NEWLY_AVAILABLE;
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
MTR_NEWLY_DEPRECATED("Please use MTRUnitTestingClusterTestEmitTestFabricScopedEventRequestParams")
@interface MTRTestClusterClusterTestEmitTestFabricScopedEventRequestParams
    : MTRUnitTestingClusterTestEmitTestFabricScopedEventRequestParams
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRFaultInjectionClusterFailAtFaultParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull type API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull id API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull numCallsToSkip API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull numCallsToFail API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull takeMutex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRFaultInjectionClusterFailRandomlyAtFaultParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull type API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull id API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull percentage API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
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
@end

NS_ASSUME_NONNULL_END
