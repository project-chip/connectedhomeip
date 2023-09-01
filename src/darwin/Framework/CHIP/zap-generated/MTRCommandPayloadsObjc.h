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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupsClusterAddGroupParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

@interface MTRGroupsClusterAddGroupParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED(
    "Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupsClusterAddGroupResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRGroupsClusterAddGroupResponseParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED(
    "Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupsClusterViewGroupParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

@interface MTRGroupsClusterViewGroupParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED(
    "Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupsClusterViewGroupResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRGroupsClusterViewGroupResponseParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED(
    "Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupsClusterRemoveGroupParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

@interface MTRGroupsClusterRemoveGroupParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED(
    "Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupsClusterRemoveGroupResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRGroupsClusterRemoveGroupResponseParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED(
    "Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupsClusterAddGroupIfIdentifyingParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

@interface MTRGroupsClusterAddGroupIfIdentifyingParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED(
    "Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterAddSceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

@interface MTRScenesClusterAddSceneParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED(
    "Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId MTR_DEPRECATED(
    "Please use sceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterAddSceneResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRScenesClusterAddSceneResponseParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED(
    "Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId MTR_DEPRECATED(
    "Please use sceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterViewSceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

@interface MTRScenesClusterViewSceneParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED(
    "Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId MTR_DEPRECATED(
    "Please use sceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterViewSceneResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRScenesClusterViewSceneResponseParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED(
    "Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId MTR_DEPRECATED(
    "Please use sceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterRemoveSceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

@interface MTRScenesClusterRemoveSceneParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED(
    "Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId MTR_DEPRECATED(
    "Please use sceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterRemoveSceneResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRScenesClusterRemoveSceneResponseParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED(
    "Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId MTR_DEPRECATED(
    "Please use sceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterRemoveAllScenesParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

@interface MTRScenesClusterRemoveAllScenesParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED(
    "Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterRemoveAllScenesResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRScenesClusterRemoveAllScenesResponseParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED(
    "Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterStoreSceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

@interface MTRScenesClusterStoreSceneParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED(
    "Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId MTR_DEPRECATED(
    "Please use sceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterStoreSceneResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRScenesClusterStoreSceneResponseParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED(
    "Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId MTR_DEPRECATED(
    "Please use sceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterRecallSceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

@interface MTRScenesClusterRecallSceneParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED(
    "Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId MTR_DEPRECATED(
    "Please use sceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterGetSceneMembershipParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

@interface MTRScenesClusterGetSceneMembershipParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED(
    "Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterGetSceneMembershipResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable capacity API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRScenesClusterGetSceneMembershipResponseParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED(
    "Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterEnhancedAddSceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

@interface MTRScenesClusterEnhancedAddSceneParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED(
    "Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId MTR_DEPRECATED(
    "Please use sceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterEnhancedAddSceneResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRScenesClusterEnhancedAddSceneResponseParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED(
    "Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId MTR_DEPRECATED(
    "Please use sceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterEnhancedViewSceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

@interface MTRScenesClusterEnhancedViewSceneParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED(
    "Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId MTR_DEPRECATED(
    "Please use sceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterEnhancedViewSceneResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRScenesClusterEnhancedViewSceneResponseParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED(
    "Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId MTR_DEPRECATED(
    "Please use sceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterCopySceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull mode API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupIdentifierFrom API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneIdentifierFrom API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull groupIdentifierTo API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneIdentifierTo API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

@interface MTRScenesClusterCopySceneParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupIdFrom MTR_DEPRECATED(
    "Please use groupIdentifierFrom", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneIdFrom MTR_DEPRECATED(
    "Please use sceneIdentifierFrom", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull groupIdTo MTR_DEPRECATED(
    "Please use groupIdentifierTo", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneIdTo MTR_DEPRECATED(
    "Please use sceneIdentifierTo", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterCopySceneResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupIdentifierFrom API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneIdentifierFrom API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRScenesClusterCopySceneResponseParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupIdFrom MTR_DEPRECATED(
    "Please use groupIdentifierFrom", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneIdFrom MTR_DEPRECATED(
    "Please use sceneIdentifierFrom", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROnOffClusterOffWithEffectParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull effectIdentifier API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

@interface MTROnOffClusterOffWithEffectParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull effectId MTR_DEPRECATED(
    "Please use effectIdentifier", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_DEPRECATED("This command has been removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTROTASoftwareUpdateProviderClusterQueryImageParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull vendorID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull productID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSArray * _Nonnull protocolsSupported API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable hardwareVersion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSString * _Nullable location API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable requestorCanConsent API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    ;

@property (nonatomic, copy) NSData * _Nullable metadataForProvider API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_DEPRECATED("Please use MTROTASoftwareUpdateProviderClusterQueryImageParams", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTROtaSoftwareUpdateProviderClusterQueryImageParams : MTROTASoftwareUpdateProviderClusterQueryImageParams
@end

@interface MTROTASoftwareUpdateProviderClusterQueryImageParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull vendorId MTR_DEPRECATED(
    "Please use vendorID", ios(16.4, 16.4), macos(13.3, 13.3), watchos(9.4, 9.4), tvos(16.4, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull productId MTR_DEPRECATED(
    "Please use productID", ios(16.4, 16.4), macos(13.3, 13.3), watchos(9.4, 9.4), tvos(16.4, 16.4));
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTROTASoftwareUpdateProviderClusterQueryImageResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable delayedActionTime API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSString * _Nullable imageURI API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable softwareVersion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy)
    NSString * _Nullable softwareVersionString API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSData * _Nullable updateToken API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable userConsentNeeded API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSData * _Nullable metadataForRequestor API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_DEPRECATED("Please use MTROTASoftwareUpdateProviderClusterQueryImageResponseParams", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTROtaSoftwareUpdateProviderClusterQueryImageResponseParams : MTROTASoftwareUpdateProviderClusterQueryImageResponseParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTROTASoftwareUpdateProviderClusterApplyUpdateRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull updateToken API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy, getter=getNewVersion)
    NSNumber * _Nonnull newVersion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_DEPRECATED("Please use MTROTASoftwareUpdateProviderClusterApplyUpdateRequestParams", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTROtaSoftwareUpdateProviderClusterApplyUpdateRequestParams : MTROTASoftwareUpdateProviderClusterApplyUpdateRequestParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull action API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull delayedActionTime API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_DEPRECATED("Please use MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams
    : MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTROTASoftwareUpdateProviderClusterNotifyUpdateAppliedParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull updateToken API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_DEPRECATED("Please use MTROTASoftwareUpdateProviderClusterNotifyUpdateAppliedParams", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTROtaSoftwareUpdateProviderClusterNotifyUpdateAppliedParams
    : MTROTASoftwareUpdateProviderClusterNotifyUpdateAppliedParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTROTASoftwareUpdateRequestorClusterAnnounceOTAProviderParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull providerNodeID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull vendorID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull announcementReason API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSData * _Nullable metadataForNode API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull endpoint API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterAnnounceOTAProviderParams", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTROtaSoftwareUpdateRequestorClusterAnnounceOtaProviderParams
    : MTROTASoftwareUpdateRequestorClusterAnnounceOTAProviderParams
@end

@interface MTROTASoftwareUpdateRequestorClusterAnnounceOTAProviderParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull providerNodeId MTR_DEPRECATED(
    "Please use providerNodeID", ios(16.4, 16.4), macos(13.3, 13.3), watchos(9.4, 9.4), tvos(16.4, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull vendorId MTR_DEPRECATED(
    "Please use vendorID", ios(16.4, 16.4), macos(13.3, 13.3), watchos(9.4, 9.4), tvos(16.4, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDiagnosticLogsClusterRetrieveLogsRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull intent API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull requestedProtocol API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy)
    NSString * _Nullable transferFileDesignator API_AVAILABLE(ios(16.5), macos(13.4), watchos(9.5), tvos(16.5));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDiagnosticLogsClusterRetrieveLogsResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSData * _Nonnull logContent API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable utcTimeStamp API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable timeSinceBoot API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRDiagnosticLogsClusterRetrieveLogsResponseParams (Deprecated)

@property (nonatomic, copy) NSData * _Nonnull content MTR_DEPRECATED(
    "Please use logContent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nullable timeStamp MTR_DEPRECATED(
    "Please use utcTimeStamp", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRAdministratorCommissioningClusterOpenCommissioningWindowParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull commissioningTimeout API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
    ;

@property (nonatomic, copy) NSData * _Nonnull pakePasscodeVerifier API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

@interface MTRAdministratorCommissioningClusterOpenCommissioningWindowParams (Deprecated)

@property (nonatomic, copy) NSData * _Nonnull pakeVerifier MTR_DEPRECATED(
    "Please use pakePasscodeVerifier", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROperationalCredentialsClusterAttestationResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull attestationElements API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSData * _Nonnull attestationSignature API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTROperationalCredentialsClusterAttestationResponseParams (Deprecated)

@property (nonatomic, copy) NSData * _Nonnull signature MTR_DEPRECATED(
    "Please use attestationSignature", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROperationalCredentialsClusterAddTrustedRootCertificateParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull rootCACertificate API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

@interface MTROperationalCredentialsClusterAddTrustedRootCertificateParams (Deprecated)

@property (nonatomic, copy) NSData * _Nonnull rootCertificate MTR_DEPRECATED(
    "Please use rootCACertificate", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterSetUserParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull operationType API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull userIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable userName API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable userUniqueID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

@interface MTRDoorLockClusterSetUserParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nullable userUniqueId MTR_DEPRECATED(
    "Please use userUniqueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterGetUserResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull userIndex API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable userName API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable userUniqueID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRDoorLockClusterGetUserResponseParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nullable userUniqueId MTR_DEPRECATED(
    "Please use userUniqueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterMoveToColorTemperatureParams : NSObject <NSCopying>

@property (nonatomic, copy)
    NSNumber * _Nonnull colorTemperatureMireds API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

@interface MTRColorControlClusterMoveToColorTemperatureParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull colorTemperature MTR_DEPRECATED(
    "Please use colorTemperatureMireds", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRMediaPlaybackClusterStopParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_DEPRECATED(
    "Please use MTRMediaPlaybackClusterStopParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRMediaPlaybackClusterStopPlaybackParams : MTRMediaPlaybackClusterStopParams
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRContentLauncherClusterLaunchContentParams : NSObject <NSCopying>

@property (nonatomic, copy)
    MTRContentLauncherClusterContentSearchStruct * _Nonnull search API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRContentLauncherClusterLaunchURLParams : NSObject <NSCopying>

@property (nonatomic, copy) NSString * _Nonnull contentURL API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable displayString API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) MTRContentLauncherClusterBrandingInformationStruct * _Nullable brandingInformation API_AVAILABLE(
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRContentLauncherClusterLauncherResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSString * _Nullable data API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_DEPRECATED("Please use MTRContentLauncherClusterLauncherResponseParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4),
    tvos(16.1, 16.4))
@interface MTRContentLauncherClusterLaunchResponseParams : MTRContentLauncherClusterLauncherResponseParams
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRApplicationLauncherClusterLaunchAppParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRApplicationLauncherClusterApplicationStruct * _Nullable application API_AVAILABLE(
    ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRApplicationLauncherClusterStopAppParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRApplicationLauncherClusterApplicationStruct * _Nullable application API_AVAILABLE(
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRApplicationLauncherClusterHideAppParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRApplicationLauncherClusterApplicationStruct * _Nullable application API_AVAILABLE(
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRApplicationLauncherClusterLauncherResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRClientMonitoringClusterRegisterClientMonitoringParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull clientNodeId MTR_PROVISIONALLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull iCid MTR_PROVISIONALLY_AVAILABLE;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRClientMonitoringClusterUnregisterClientMonitoringParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull clientNodeId MTR_PROVISIONALLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull iCid MTR_PROVISIONALLY_AVAILABLE;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRClientMonitoringClusterStayAwakeRequestParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_DEPRECATED(
    "Please use MTRUnitTestingClusterTestParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestParams : MTRUnitTestingClusterTestParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestSpecificResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull returnValue API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestSpecificResponseParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4),
    tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestSpecificResponseParams : MTRUnitTestingClusterTestSpecificResponseParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_DEPRECATED(
    "Please use MTRUnitTestingClusterTestNotHandledParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestNotHandledParams : MTRUnitTestingClusterTestNotHandledParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestAddArgumentsResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull returnValue API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestAddArgumentsResponseParams", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestAddArgumentsResponseParams : MTRUnitTestingClusterTestAddArgumentsResponseParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_DEPRECATED(
    "Please use MTRUnitTestingClusterTestSpecificParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestSpecificParams : MTRUnitTestingClusterTestSpecificParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestSimpleArgumentResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull returnValue API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestSimpleArgumentResponseParams", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestSimpleArgumentResponseParams : MTRUnitTestingClusterTestSimpleArgumentResponseParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestUnknownCommandParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4),
    tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestUnknownCommandParams : MTRUnitTestingClusterTestUnknownCommandParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestStructArrayArgumentResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull arg1 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSArray * _Nonnull arg2 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSArray * _Nonnull arg3 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSArray * _Nonnull arg4 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull arg5 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull arg6 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestStructArrayArgumentResponseParams", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestStructArrayArgumentResponseParams : MTRUnitTestingClusterTestStructArrayArgumentResponseParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestAddArgumentsParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull arg1 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull arg2 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestAddArgumentsParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4),
    tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestAddArgumentsParams : MTRUnitTestingClusterTestAddArgumentsParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestListInt8UReverseResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull arg1 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestListInt8UReverseResponseParams", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestListInt8UReverseResponseParams : MTRUnitTestingClusterTestListInt8UReverseResponseParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestSimpleArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull arg1 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestSimpleArgumentRequestParams", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestSimpleArgumentRequestParams : MTRUnitTestingClusterTestSimpleArgumentRequestParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestEnumsResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull arg1 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull arg2 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestEnumsResponseParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4),
    tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestEnumsResponseParams : MTRUnitTestingClusterTestEnumsResponseParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestStructArrayArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull arg1 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSArray * _Nonnull arg2 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSArray * _Nonnull arg3 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSArray * _Nonnull arg4 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull arg5 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull arg6 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestStructArrayArgumentRequestParams", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestStructArrayArgumentRequestParams : MTRUnitTestingClusterTestStructArrayArgumentRequestParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestNullableOptionalResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull wasPresent API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable wasNull API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable value API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable originalValue API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestNullableOptionalResponseParams", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestNullableOptionalResponseParams : MTRUnitTestingClusterTestNullableOptionalResponseParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestStructArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy)
    MTRUnitTestingClusterSimpleStruct * _Nonnull arg1 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestStructArgumentRequestParams", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestStructArgumentRequestParams : MTRUnitTestingClusterTestStructArgumentRequestParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestComplexNullableOptionalResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull nullableIntWasNull API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable nullableIntValue API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy)
    NSNumber * _Nonnull optionalIntWasPresent API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable optionalIntValue API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy)
    NSNumber * _Nonnull nullableOptionalIntWasPresent API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy)
    NSNumber * _Nullable nullableOptionalIntWasNull API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy)
    NSNumber * _Nullable nullableOptionalIntValue API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy)
    NSNumber * _Nonnull nullableStringWasNull API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSString * _Nullable nullableStringValue API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    ;

@property (nonatomic, copy)
    NSNumber * _Nonnull optionalStringWasPresent API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSString * _Nullable optionalStringValue API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    ;

@property (nonatomic, copy)
    NSNumber * _Nonnull nullableOptionalStringWasPresent API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy)
    NSNumber * _Nullable nullableOptionalStringWasNull API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy)
    NSString * _Nullable nullableOptionalStringValue API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy)
    NSNumber * _Nonnull nullableStructWasNull API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable nullableStructValue API_AVAILABLE(
    ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy)
    NSNumber * _Nonnull optionalStructWasPresent API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable optionalStructValue API_AVAILABLE(
    ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy)
    NSNumber * _Nonnull nullableOptionalStructWasPresent API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy)
    NSNumber * _Nullable nullableOptionalStructWasNull API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable nullableOptionalStructValue API_AVAILABLE(
    ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull nullableListWasNull API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSArray * _Nullable nullableListValue API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy)
    NSNumber * _Nonnull optionalListWasPresent API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSArray * _Nullable optionalListValue API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy)
    NSNumber * _Nonnull nullableOptionalListWasPresent API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy)
    NSNumber * _Nullable nullableOptionalListWasNull API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy)
    NSArray * _Nullable nullableOptionalListValue API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestComplexNullableOptionalResponseParams", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestComplexNullableOptionalResponseParams
    : MTRUnitTestingClusterTestComplexNullableOptionalResponseParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestNestedStructArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy)
    MTRUnitTestingClusterNestedStruct * _Nonnull arg1 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestNestedStructArgumentRequestParams", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestNestedStructArgumentRequestParams : MTRUnitTestingClusterTestNestedStructArgumentRequestParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterBooleanResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull value API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterBooleanResponseParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4),
    tvos(16.1, 16.4))
@interface MTRTestClusterClusterBooleanResponseParams : MTRUnitTestingClusterBooleanResponseParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestListStructArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull arg1 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestListStructArgumentRequestParams", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestListStructArgumentRequestParams : MTRUnitTestingClusterTestListStructArgumentRequestParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterSimpleStructResponseParams : NSObject <NSCopying>

@property (nonatomic, copy)
    MTRUnitTestingClusterSimpleStruct * _Nonnull arg1 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterSimpleStructResponseParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4),
    tvos(16.1, 16.4))
@interface MTRTestClusterClusterSimpleStructResponseParams : MTRUnitTestingClusterSimpleStructResponseParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestListInt8UArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull arg1 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestListInt8UArgumentRequestParams", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestListInt8UArgumentRequestParams : MTRUnitTestingClusterTestListInt8UArgumentRequestParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestEmitTestEventResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull value API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestEmitTestEventResponseParams", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestEmitTestEventResponseParams : MTRUnitTestingClusterTestEmitTestEventResponseParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestNestedStructListArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy)
    MTRUnitTestingClusterNestedStructList * _Nonnull arg1 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestNestedStructListArgumentRequestParams", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestNestedStructListArgumentRequestParams
    : MTRUnitTestingClusterTestNestedStructListArgumentRequestParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestEmitTestFabricScopedEventResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull value API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable)
    NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestEmitTestFabricScopedEventResponseParams", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestEmitTestFabricScopedEventResponseParams
    : MTRUnitTestingClusterTestEmitTestFabricScopedEventResponseParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestListNestedStructListArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull arg1 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestListNestedStructListArgumentRequestParams", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestListNestedStructListArgumentRequestParams
    : MTRUnitTestingClusterTestListNestedStructListArgumentRequestParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestListInt8UReverseRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull arg1 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestListInt8UReverseRequestParams", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestListInt8UReverseRequestParams : MTRUnitTestingClusterTestListInt8UReverseRequestParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestEnumsRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull arg1 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull arg2 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestEnumsRequestParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4),
    tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestEnumsRequestParams : MTRUnitTestingClusterTestEnumsRequestParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestNullableOptionalRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nullable arg1 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestNullableOptionalRequestParams", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestNullableOptionalRequestParams : MTRUnitTestingClusterTestNullableOptionalRequestParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestComplexNullableOptionalRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nullable nullableInt API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable optionalInt API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalInt API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    ;

@property (nonatomic, copy) NSString * _Nullable nullableString API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSString * _Nullable optionalString API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy)
    NSString * _Nullable nullableOptionalString API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy)
    MTRUnitTestingClusterSimpleStruct * _Nullable nullableStruct API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy)
    MTRUnitTestingClusterSimpleStruct * _Nullable optionalStruct API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable nullableOptionalStruct API_AVAILABLE(
    ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSArray * _Nullable nullableList API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSArray * _Nullable optionalList API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSArray * _Nullable nullableOptionalList API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestComplexNullableOptionalRequestParams", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestComplexNullableOptionalRequestParams
    : MTRUnitTestingClusterTestComplexNullableOptionalRequestParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterSimpleStructEchoRequestParams : NSObject <NSCopying>

@property (nonatomic, copy)
    MTRUnitTestingClusterSimpleStruct * _Nonnull arg1 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterSimpleStructEchoRequestParams", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterSimpleStructEchoRequestParams : MTRUnitTestingClusterSimpleStructEchoRequestParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTimedInvokeRequestParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4),
    tvos(16.1, 16.4))
@interface MTRTestClusterClusterTimedInvokeRequestParams : MTRUnitTestingClusterTimedInvokeRequestParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestSimpleOptionalArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nullable arg1 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestSimpleOptionalArgumentRequestParams", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestSimpleOptionalArgumentRequestParams
    : MTRUnitTestingClusterTestSimpleOptionalArgumentRequestParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestEmitTestEventRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull arg1 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull arg2 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull arg3 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestEmitTestEventRequestParams", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestEmitTestEventRequestParams : MTRUnitTestingClusterTestEmitTestEventRequestParams
@end

API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestEmitTestFabricScopedEventRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull arg1 API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestEmitTestFabricScopedEventRequestParams", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
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

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

NS_ASSUME_NONNULL_END
