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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRIdentifyClusterIdentifyParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull identifyTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRIdentifyClusterTriggerEffectParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull effectIdentifier MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull effectVariant MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupsClusterAddGroupParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSString * _Nonnull groupName MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED("Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupsClusterAddGroupResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRGroupsClusterAddGroupResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

@interface MTRGroupsClusterAddGroupResponseParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED("Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupsClusterViewGroupParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED("Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupsClusterViewGroupResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSString * _Nonnull groupName MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRGroupsClusterViewGroupResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

@interface MTRGroupsClusterViewGroupResponseParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED("Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupsClusterGetGroupMembershipParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull groupList MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupsClusterGetGroupMembershipResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nullable capacity MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSArray * _Nonnull groupList MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRGroupsClusterGetGroupMembershipResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupsClusterRemoveGroupParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED("Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupsClusterRemoveGroupResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRGroupsClusterRemoveGroupResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

@interface MTRGroupsClusterRemoveGroupResponseParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED("Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupsClusterAddGroupIfIdentifyingParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSString * _Nonnull groupName MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED("Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterAddSceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nonnull sceneName MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSArray * _Nonnull extensionFieldSets MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED("Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId MTR_DEPRECATED("Please use sceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterAddSceneResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRScenesClusterAddSceneResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

@interface MTRScenesClusterAddSceneResponseParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED("Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId MTR_DEPRECATED("Please use sceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterViewSceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED("Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId MTR_DEPRECATED("Please use sceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterViewSceneResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable transitionTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable sceneName MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSArray * _Nullable extensionFieldSets MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRScenesClusterViewSceneResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

@interface MTRScenesClusterViewSceneResponseParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED("Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId MTR_DEPRECATED("Please use sceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterRemoveSceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED("Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId MTR_DEPRECATED("Please use sceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterRemoveSceneResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRScenesClusterRemoveSceneResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

@interface MTRScenesClusterRemoveSceneResponseParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED("Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId MTR_DEPRECATED("Please use sceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterRemoveAllScenesParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED("Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterRemoveAllScenesResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRScenesClusterRemoveAllScenesResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

@interface MTRScenesClusterRemoveAllScenesResponseParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED("Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterStoreSceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED("Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId MTR_DEPRECATED("Please use sceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterStoreSceneResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRScenesClusterStoreSceneResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

@interface MTRScenesClusterStoreSceneResponseParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED("Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId MTR_DEPRECATED("Please use sceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterRecallSceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable transitionTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED("Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId MTR_DEPRECATED("Please use sceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterGetSceneMembershipParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED("Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterGetSceneMembershipResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable capacity MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSArray * _Nullable sceneList MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRScenesClusterGetSceneMembershipResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

@interface MTRScenesClusterGetSceneMembershipResponseParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED("Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterEnhancedAddSceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nonnull sceneName MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSArray * _Nonnull extensionFieldSets MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED("Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId MTR_DEPRECATED("Please use sceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterEnhancedAddSceneResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRScenesClusterEnhancedAddSceneResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

@interface MTRScenesClusterEnhancedAddSceneResponseParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED("Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId MTR_DEPRECATED("Please use sceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterEnhancedViewSceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED("Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId MTR_DEPRECATED("Please use sceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterEnhancedViewSceneResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable transitionTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable sceneName MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSArray * _Nullable extensionFieldSets MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRScenesClusterEnhancedViewSceneResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

@interface MTRScenesClusterEnhancedViewSceneResponseParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupId MTR_DEPRECATED("Please use groupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneId MTR_DEPRECATED("Please use sceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterCopySceneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull mode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupIdentifierFrom MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneIdentifierFrom MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull groupIdentifierTo MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneIdentifierTo MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

@property (nonatomic, copy) NSNumber * _Nonnull groupIdFrom MTR_DEPRECATED("Please use groupIdentifierFrom", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneIdFrom MTR_DEPRECATED("Please use sceneIdentifierFrom", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull groupIdTo MTR_DEPRECATED("Please use groupIdentifierTo", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneIdTo MTR_DEPRECATED("Please use sceneIdentifierTo", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRScenesClusterCopySceneResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull groupIdentifierFrom MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneIdentifierFrom MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRScenesClusterCopySceneResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

@interface MTRScenesClusterCopySceneResponseParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull groupIdFrom MTR_DEPRECATED("Please use groupIdentifierFrom", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull sceneIdFrom MTR_DEPRECATED("Please use sceneIdentifierFrom", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROnOffClusterOffWithEffectParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull effectIdentifier MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull effectVariant MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

@property (nonatomic, copy) NSNumber * _Nonnull effectId MTR_DEPRECATED("Please use effectIdentifier", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROnOffClusterOnWithTimedOffParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull onOffControl MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull onTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull offWaitTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRLevelControlClusterMoveToLevelParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull level MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable transitionTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRLevelControlClusterMoveParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull moveMode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable rate MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRLevelControlClusterStepParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull stepMode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull stepSize MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable transitionTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRLevelControlClusterStopParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRLevelControlClusterMoveToLevelWithOnOffParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull level MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable transitionTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRLevelControlClusterMoveWithOnOffParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull moveMode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable rate MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRLevelControlClusterStepWithOnOffParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull stepMode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull stepSize MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable transitionTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRLevelControlClusterStopWithOnOffParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRLevelControlClusterMoveToClosestFrequencyParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull frequency MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterInstantActionParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable invokeID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterInstantActionWithTransitionParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable invokeID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterStartActionParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable invokeID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterStartActionWithDurationParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable invokeID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull duration MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterStopActionParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable invokeID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterPauseActionParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable invokeID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterPauseActionWithDurationParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable invokeID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull duration MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterResumeActionParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable invokeID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterEnableActionParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable invokeID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterEnableActionWithDurationParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable invokeID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull duration MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterDisableActionParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable invokeID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRActionsClusterDisableActionWithDurationParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull actionID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable invokeID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull duration MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTROTASoftwareUpdateProviderClusterQueryImageParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull vendorID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull productID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSArray * _Nonnull protocolsSupported MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable hardwareVersion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSString * _Nullable location MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable requestorCanConsent MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSData * _Nullable metadataForProvider MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_DEPRECATED("Please use MTROTASoftwareUpdateProviderClusterQueryImageParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTROtaSoftwareUpdateProviderClusterQueryImageParams : MTROTASoftwareUpdateProviderClusterQueryImageParams

@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion MTR_DEPRECATED("The softwareVersion field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSArray * _Nonnull protocolsSupported MTR_DEPRECATED("The protocolsSupported field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nullable hardwareVersion MTR_DEPRECATED("The hardwareVersion field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSString * _Nullable location MTR_DEPRECATED("The location field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nullable requestorCanConsent MTR_DEPRECATED("The requestorCanConsent field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSData * _Nullable metadataForProvider MTR_DEPRECATED("The metadataForProvider field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

@interface MTROTASoftwareUpdateProviderClusterQueryImageParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull vendorId MTR_DEPRECATED("Please use vendorID", ios(16.4, 16.4), macos(13.3, 13.3), watchos(9.4, 9.4), tvos(16.4, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull productId MTR_DEPRECATED("Please use productID", ios(16.4, 16.4), macos(13.3, 13.3), watchos(9.4, 9.4), tvos(16.4, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTROTASoftwareUpdateProviderClusterQueryImageResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable delayedActionTime MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSString * _Nullable imageURI MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable softwareVersion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSString * _Nullable softwareVersionString MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSData * _Nullable updateToken MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable userConsentNeeded MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSData * _Nullable metadataForRequestor MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTROTASoftwareUpdateProviderClusterQueryImageResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_DEPRECATED("Please use MTROTASoftwareUpdateProviderClusterQueryImageResponseParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTROtaSoftwareUpdateProviderClusterQueryImageResponseParams : MTROTASoftwareUpdateProviderClusterQueryImageResponseParams

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_DEPRECATED("The status field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nullable delayedActionTime MTR_DEPRECATED("The delayedActionTime field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSString * _Nullable imageURI MTR_DEPRECATED("The imageURI field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nullable softwareVersion MTR_DEPRECATED("The softwareVersion field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSString * _Nullable softwareVersionString MTR_DEPRECATED("The softwareVersionString field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSData * _Nullable updateToken MTR_DEPRECATED("The updateToken field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nullable userConsentNeeded MTR_DEPRECATED("The userConsentNeeded field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSData * _Nullable metadataForRequestor MTR_DEPRECATED("The metadataForRequestor field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTROTASoftwareUpdateProviderClusterApplyUpdateRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull updateToken MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy, getter=getNewVersion) NSNumber * _Nonnull newVersion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_DEPRECATED("Please use MTROTASoftwareUpdateProviderClusterApplyUpdateRequestParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTROtaSoftwareUpdateProviderClusterApplyUpdateRequestParams : MTROTASoftwareUpdateProviderClusterApplyUpdateRequestParams

@property (nonatomic, copy) NSData * _Nonnull updateToken MTR_DEPRECATED("The updateToken field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy, getter=getNewVersion) NSNumber * _Nonnull newVersion MTR_DEPRECATED("The newVersion field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull action MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull delayedActionTime MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_DEPRECATED("Please use MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams : MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams

@property (nonatomic, copy) NSNumber * _Nonnull action MTR_DEPRECATED("The action field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull delayedActionTime MTR_DEPRECATED("The delayedActionTime field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTROTASoftwareUpdateProviderClusterNotifyUpdateAppliedParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull updateToken MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_DEPRECATED("Please use MTROTASoftwareUpdateProviderClusterNotifyUpdateAppliedParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTROtaSoftwareUpdateProviderClusterNotifyUpdateAppliedParams : MTROTASoftwareUpdateProviderClusterNotifyUpdateAppliedParams

@property (nonatomic, copy) NSData * _Nonnull updateToken MTR_DEPRECATED("The updateToken field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull softwareVersion MTR_DEPRECATED("The softwareVersion field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTROTASoftwareUpdateRequestorClusterAnnounceOTAProviderParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull providerNodeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull vendorID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull announcementReason MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSData * _Nullable metadataForNode MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull endpoint MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorClusterAnnounceOTAProviderParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTROtaSoftwareUpdateRequestorClusterAnnounceOtaProviderParams : MTROTASoftwareUpdateRequestorClusterAnnounceOTAProviderParams

@property (nonatomic, copy) NSNumber * _Nonnull announcementReason MTR_DEPRECATED("The announcementReason field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSData * _Nullable metadataForNode MTR_DEPRECATED("The metadataForNode field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull endpoint MTR_DEPRECATED("The endpoint field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

@interface MTROTASoftwareUpdateRequestorClusterAnnounceOTAProviderParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nonnull providerNodeId MTR_DEPRECATED("Please use providerNodeID", ios(16.4, 16.4), macos(13.3, 13.3), watchos(9.4, 9.4), tvos(16.4, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull vendorId MTR_DEPRECATED("Please use vendorID", ios(16.4, 16.4), macos(13.3, 13.3), watchos(9.4, 9.4), tvos(16.4, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGeneralCommissioningClusterArmFailSafeParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull expiryLengthSeconds MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull breadcrumb MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGeneralCommissioningClusterArmFailSafeResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull errorCode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nonnull debugText MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRGeneralCommissioningClusterArmFailSafeResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGeneralCommissioningClusterSetRegulatoryConfigParams : NSObject <NSCopying>

@property (nonatomic, copy, getter=getNewRegulatoryConfig) NSNumber * _Nonnull newRegulatoryConfig MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nonnull countryCode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull breadcrumb MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGeneralCommissioningClusterSetRegulatoryConfigResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull errorCode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nonnull debugText MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRGeneralCommissioningClusterSetRegulatoryConfigResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGeneralCommissioningClusterCommissioningCompleteResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull errorCode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nonnull debugText MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRGeneralCommissioningClusterCommissioningCompleteResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRNetworkCommissioningClusterScanNetworksParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nullable ssid MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable breadcrumb MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRNetworkCommissioningClusterScanNetworksResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull networkingStatus MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable debugText MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSArray * _Nullable wiFiScanResults MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSArray * _Nullable threadScanResults MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRNetworkCommissioningClusterScanNetworksResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRNetworkCommissioningClusterAddOrUpdateWiFiNetworkParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull ssid MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSData * _Nonnull credentials MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable breadcrumb MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRNetworkCommissioningClusterAddOrUpdateThreadNetworkParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull operationalDataset MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable breadcrumb MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRNetworkCommissioningClusterRemoveNetworkParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull networkID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable breadcrumb MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRNetworkCommissioningClusterNetworkConfigResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull networkingStatus MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable debugText MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable networkIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRNetworkCommissioningClusterNetworkConfigResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRNetworkCommissioningClusterConnectNetworkParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull networkID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable breadcrumb MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRNetworkCommissioningClusterConnectNetworkResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull networkingStatus MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable debugText MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable errorValue MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRNetworkCommissioningClusterConnectNetworkResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRNetworkCommissioningClusterReorderNetworkParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull networkID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull networkIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable breadcrumb MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDiagnosticLogsClusterRetrieveLogsRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull intent MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull requestedProtocol MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable transferFileDesignator MTR_AVAILABLE(ios(16.5), macos(13.4), watchos(9.5), tvos(16.5));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDiagnosticLogsClusterRetrieveLogsResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSData * _Nonnull logContent MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable utcTimeStamp MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable timeSinceBoot MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRDiagnosticLogsClusterRetrieveLogsResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

@interface MTRDiagnosticLogsClusterRetrieveLogsResponseParams (Deprecated)

@property (nonatomic, copy) NSData * _Nonnull content MTR_DEPRECATED("Please use logContent", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nullable timeStamp MTR_DEPRECATED("Please use utcTimeStamp", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGeneralDiagnosticsClusterTestEventTriggerParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull enableKey MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull eventTrigger MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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

MTR_AVAILABLE(ios(17.2), macos(14.2), watchos(10.2), tvos(17.2))
@interface MTRTimeSynchronizationClusterSetUTCTimeParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull utcTime MTR_AVAILABLE(ios(17.2), macos(14.2), watchos(10.2), tvos(17.2));

@property (nonatomic, copy) NSNumber * _Nonnull granularity MTR_AVAILABLE(ios(17.2), macos(14.2), watchos(10.2), tvos(17.2));

@property (nonatomic, copy) NSNumber * _Nullable timeSource MTR_AVAILABLE(ios(17.2), macos(14.2), watchos(10.2), tvos(17.2));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRTimeSynchronizationClusterSetUtcTimeParams : MTRTimeSynchronizationClusterSetUTCTimeParams

@property (nonatomic, copy) NSNumber * _Nonnull utcTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull granularity MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable timeSource MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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
@interface MTRTimeSynchronizationClusterSetTrustedTimeSourceParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRTimeSynchronizationClusterFabricScopedTrustedTimeSourceStruct * _Nullable trustedTimeSource MTR_PROVISIONALLY_AVAILABLE;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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
@interface MTRTimeSynchronizationClusterSetTimeZoneParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull timeZone MTR_PROVISIONALLY_AVAILABLE;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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
@interface MTRTimeSynchronizationClusterSetTimeZoneResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull dstOffsetRequired MTR_PROVISIONALLY_AVAILABLE;

/**
 * Initialize an MTRTimeSynchronizationClusterSetTimeZoneResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRTimeSynchronizationClusterSetDSTOffsetParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull dstOffset MTR_PROVISIONALLY_AVAILABLE;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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
@interface MTRTimeSynchronizationClusterSetDefaultNTPParams : NSObject <NSCopying>

@property (nonatomic, copy) NSString * _Nullable defaultNTP MTR_PROVISIONALLY_AVAILABLE;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRAdministratorCommissioningClusterOpenCommissioningWindowParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull commissioningTimeout MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSData * _Nonnull pakePasscodeVerifier MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull discriminator MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull iterations MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSData * _Nonnull salt MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

@property (nonatomic, copy) NSData * _Nonnull pakeVerifier MTR_DEPRECATED("Please use pakePasscodeVerifier", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRAdministratorCommissioningClusterOpenBasicCommissioningWindowParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull commissioningTimeout MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROperationalCredentialsClusterAttestationRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull attestationNonce MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROperationalCredentialsClusterAttestationResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull attestationElements MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSData * _Nonnull attestationSignature MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTROperationalCredentialsClusterAttestationResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

@interface MTROperationalCredentialsClusterAttestationResponseParams (Deprecated)

@property (nonatomic, copy) NSData * _Nonnull signature MTR_DEPRECATED("Please use attestationSignature", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROperationalCredentialsClusterCertificateChainRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull certificateType MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROperationalCredentialsClusterCertificateChainResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull certificate MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTROperationalCredentialsClusterCertificateChainResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROperationalCredentialsClusterCSRRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull csrNonce MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable isForUpdateNOC MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROperationalCredentialsClusterCSRResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull nocsrElements MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSData * _Nonnull attestationSignature MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTROperationalCredentialsClusterCSRResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROperationalCredentialsClusterAddNOCParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull nocValue MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSData * _Nullable icacValue MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSData * _Nonnull ipkValue MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull caseAdminSubject MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull adminVendorId MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROperationalCredentialsClusterUpdateNOCParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull nocValue MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSData * _Nullable icacValue MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROperationalCredentialsClusterNOCResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull statusCode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable fabricIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable debugText MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTROperationalCredentialsClusterNOCResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROperationalCredentialsClusterUpdateFabricLabelParams : NSObject <NSCopying>

@property (nonatomic, copy) NSString * _Nonnull label MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROperationalCredentialsClusterRemoveFabricParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull fabricIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROperationalCredentialsClusterAddTrustedRootCertificateParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nonnull rootCACertificate MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

@property (nonatomic, copy) NSData * _Nonnull rootCertificate MTR_DEPRECATED("Please use rootCACertificate", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupKeyManagementClusterKeySetWriteParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRGroupKeyManagementClusterGroupKeySetStruct * _Nonnull groupKeySet MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupKeyManagementClusterKeySetReadParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupKeySetID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupKeyManagementClusterKeySetReadResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRGroupKeyManagementClusterGroupKeySetStruct * _Nonnull groupKeySet MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRGroupKeyManagementClusterKeySetReadResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupKeyManagementClusterKeySetRemoveParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull groupKeySetID MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupKeyManagementClusterKeySetReadAllIndicesParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRGroupKeyManagementClusterKeySetReadAllIndicesResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull groupKeySetIDs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRGroupKeyManagementClusterKeySetReadAllIndicesResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRICDManagementClusterRegisterClientParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull checkInNodeID MTR_PROVISIONALLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull monitoredSubject MTR_PROVISIONALLY_AVAILABLE;

@property (nonatomic, copy) NSData * _Nonnull key MTR_PROVISIONALLY_AVAILABLE;

@property (nonatomic, copy) NSData * _Nullable verificationKey MTR_PROVISIONALLY_AVAILABLE;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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
@interface MTRICDManagementClusterRegisterClientResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull icdCounter MTR_PROVISIONALLY_AVAILABLE;

/**
 * Initialize an MTRICDManagementClusterRegisterClientResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRICDManagementClusterUnregisterClientParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull checkInNodeID MTR_PROVISIONALLY_AVAILABLE;

@property (nonatomic, copy) NSData * _Nullable verificationKey MTR_PROVISIONALLY_AVAILABLE;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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
@interface MTRICDManagementClusterStayActiveRequestParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRModeSelectClusterChangeToModeParams : NSObject <NSCopying>

@property (nonatomic, copy, getter=getNewMode) NSNumber * _Nonnull newMode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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
@interface MTRLaundryWasherModeClusterChangeToModeParams : NSObject <NSCopying>

@property (nonatomic, copy, getter=getNewMode) NSNumber * _Nonnull newMode MTR_PROVISIONALLY_AVAILABLE;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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
@interface MTRLaundryWasherModeClusterChangeToModeResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_PROVISIONALLY_AVAILABLE;

@property (nonatomic, copy) NSString * _Nullable statusText MTR_PROVISIONALLY_AVAILABLE;

/**
 * Initialize an MTRLaundryWasherModeClusterChangeToModeResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRRefrigeratorAndTemperatureControlledCabinetModeClusterChangeToModeParams : NSObject <NSCopying>

@property (nonatomic, copy, getter=getNewMode) NSNumber * _Nonnull newMode MTR_PROVISIONALLY_AVAILABLE;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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
@interface MTRRefrigeratorAndTemperatureControlledCabinetModeClusterChangeToModeResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_PROVISIONALLY_AVAILABLE;

@property (nonatomic, copy) NSString * _Nullable statusText MTR_PROVISIONALLY_AVAILABLE;

/**
 * Initialize an MTRRefrigeratorAndTemperatureControlledCabinetModeClusterChangeToModeResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRRVCRunModeClusterChangeToModeParams : NSObject <NSCopying>

@property (nonatomic, copy, getter=getNewMode) NSNumber * _Nonnull newMode MTR_PROVISIONALLY_AVAILABLE;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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
@interface MTRRVCRunModeClusterChangeToModeResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_PROVISIONALLY_AVAILABLE;

@property (nonatomic, copy) NSString * _Nullable statusText MTR_PROVISIONALLY_AVAILABLE;

/**
 * Initialize an MTRRVCRunModeClusterChangeToModeResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRRVCCleanModeClusterChangeToModeParams : NSObject <NSCopying>

@property (nonatomic, copy, getter=getNewMode) NSNumber * _Nonnull newMode MTR_PROVISIONALLY_AVAILABLE;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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
@interface MTRRVCCleanModeClusterChangeToModeResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_PROVISIONALLY_AVAILABLE;

@property (nonatomic, copy) NSString * _Nullable statusText MTR_PROVISIONALLY_AVAILABLE;

/**
 * Initialize an MTRRVCCleanModeClusterChangeToModeResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRTemperatureControlClusterSetTemperatureParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nullable targetTemperature MTR_PROVISIONALLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nullable targetTemperatureLevel MTR_PROVISIONALLY_AVAILABLE;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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
@interface MTRDishwasherModeClusterChangeToModeParams : NSObject <NSCopying>

@property (nonatomic, copy, getter=getNewMode) NSNumber * _Nonnull newMode MTR_PROVISIONALLY_AVAILABLE;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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
@interface MTRDishwasherModeClusterChangeToModeResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_PROVISIONALLY_AVAILABLE;

@property (nonatomic, copy) NSString * _Nullable statusText MTR_PROVISIONALLY_AVAILABLE;

/**
 * Initialize an MTRDishwasherModeClusterChangeToModeResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRSmokeCOAlarmClusterSelfTestRequestParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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
@interface MTRDishwasherAlarmClusterResetParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull alarms MTR_PROVISIONALLY_AVAILABLE;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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
@interface MTRDishwasherAlarmClusterModifyEnabledAlarmsParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull mask MTR_PROVISIONALLY_AVAILABLE;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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
@interface MTROperationalStateClusterPauseParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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
@interface MTROperationalStateClusterStopParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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
@interface MTROperationalStateClusterStartParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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
@interface MTROperationalStateClusterResumeParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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
@interface MTROperationalStateClusterOperationalCommandResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) MTROperationalStateClusterErrorStateStruct * _Nonnull commandResponseState MTR_PROVISIONALLY_AVAILABLE;

/**
 * Initialize an MTROperationalStateClusterOperationalCommandResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRRVCOperationalStateClusterPauseParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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
@interface MTRRVCOperationalStateClusterStopParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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
@interface MTRRVCOperationalStateClusterStartParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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
@interface MTRRVCOperationalStateClusterResumeParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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
@interface MTRRVCOperationalStateClusterOperationalCommandResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRRVCOperationalStateClusterErrorStateStruct * _Nonnull commandResponseState MTR_PROVISIONALLY_AVAILABLE;

/**
 * Initialize an MTRRVCOperationalStateClusterOperationalCommandResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRHEPAFilterMonitoringClusterResetConditionParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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
@interface MTRActivatedCarbonFilterMonitoringClusterResetConditionParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterLockDoorParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nullable pinCode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterUnlockDoorParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nullable pinCode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterUnlockWithTimeoutParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull timeout MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSData * _Nullable pinCode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterSetWeekDayScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull weekDayIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull userIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull daysMask MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull startHour MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull startMinute MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull endHour MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull endMinute MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterGetWeekDayScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull weekDayIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull userIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterGetWeekDayScheduleResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull weekDayIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull userIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable daysMask MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable startHour MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable startMinute MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable endHour MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable endMinute MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRDoorLockClusterGetWeekDayScheduleResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterClearWeekDayScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull weekDayIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull userIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterSetYearDayScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull yearDayIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull userIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull localStartTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull localEndTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterGetYearDayScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull yearDayIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull userIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterGetYearDayScheduleResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull yearDayIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull userIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable localStartTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable localEndTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRDoorLockClusterGetYearDayScheduleResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterClearYearDayScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull yearDayIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull userIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterSetHolidayScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull holidayIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull localStartTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull localEndTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull operatingMode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterGetHolidayScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull holidayIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterGetHolidayScheduleResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull holidayIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable localStartTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable localEndTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable operatingMode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRDoorLockClusterGetHolidayScheduleResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterClearHolidayScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull holidayIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterSetUserParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull operationType MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull userIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable userName MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable userUniqueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable userStatus MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable userType MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable credentialRule MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

@property (nonatomic, copy) NSNumber * _Nullable userUniqueId MTR_DEPRECATED("Please use userUniqueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterGetUserParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull userIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterGetUserResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull userIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable userName MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable userUniqueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable userStatus MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable userType MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable credentialRule MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSArray * _Nullable credentials MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable creatorFabricIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable lastModifiedFabricIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable nextUserIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRDoorLockClusterGetUserResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

@interface MTRDoorLockClusterGetUserResponseParams (Deprecated)

@property (nonatomic, copy) NSNumber * _Nullable userUniqueId MTR_DEPRECATED("Please use userUniqueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterClearUserParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull userIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterSetCredentialParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull operationType MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) MTRDoorLockClusterCredentialStruct * _Nonnull credential MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSData * _Nonnull credentialData MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable userIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable userStatus MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable userType MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterSetCredentialResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable userIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable nextCredentialIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRDoorLockClusterSetCredentialResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterGetCredentialStatusParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRDoorLockClusterCredentialStruct * _Nonnull credential MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterGetCredentialStatusResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull credentialExists MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable userIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable creatorFabricIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable lastModifiedFabricIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nullable nextCredentialIndex MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRDoorLockClusterGetCredentialStatusResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDoorLockClusterClearCredentialParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRDoorLockClusterCredentialStruct * _Nullable credential MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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
@interface MTRDoorLockClusterUnboltDoorParams : NSObject <NSCopying>

@property (nonatomic, copy) NSData * _Nullable pinCode MTR_PROVISIONALLY_AVAILABLE;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRWindowCoveringClusterGoToLiftValueParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull liftValue MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRWindowCoveringClusterGoToLiftPercentageParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull liftPercent100thsValue MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRWindowCoveringClusterGoToTiltValueParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull tiltValue MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRWindowCoveringClusterGoToTiltPercentageParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull tiltPercent100thsValue MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRBarrierControlClusterBarrierControlGoToPercentParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull percentOpen MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRThermostatClusterSetpointRaiseLowerParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull mode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull amount MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRThermostatClusterGetWeeklyScheduleResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull numberOfTransitionsForSequence MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull dayOfWeekForSequence MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull modeForSequence MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSArray * _Nonnull transitions MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRThermostatClusterGetWeeklyScheduleResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRThermostatClusterSetWeeklyScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull numberOfTransitionsForSequence MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull dayOfWeekForSequence MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull modeForSequence MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSArray * _Nonnull transitions MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRThermostatClusterGetWeeklyScheduleParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull daysToReturn MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull modeToReturn MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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

MTR_PROVISIONALLY_AVAILABLE
@interface MTRFanControlClusterStepParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull direction MTR_PROVISIONALLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nullable wrap MTR_PROVISIONALLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nullable lowestOff MTR_PROVISIONALLY_AVAILABLE;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterMoveToHueParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull hue MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull direction MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterMoveHueParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull moveMode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull rate MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterStepHueParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull stepMode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull stepSize MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterMoveToSaturationParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull saturation MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterMoveSaturationParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull moveMode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull rate MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterStepSaturationParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull stepMode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull stepSize MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterMoveToHueAndSaturationParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull hue MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull saturation MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterMoveToColorParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull colorX MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull colorY MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterMoveColorParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull rateX MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull rateY MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterStepColorParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull stepX MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull stepY MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterMoveToColorTemperatureParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull colorTemperatureMireds MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

@property (nonatomic, copy) NSNumber * _Nonnull colorTemperature MTR_DEPRECATED("Please use colorTemperatureMireds", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterEnhancedMoveToHueParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull enhancedHue MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull direction MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterEnhancedMoveHueParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull moveMode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull rate MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterEnhancedStepHueParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull stepMode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull stepSize MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterEnhancedMoveToHueAndSaturationParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull enhancedHue MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull saturation MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterColorLoopSetParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull updateFlags MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull action MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull direction MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull time MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull startHue MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterStopMoveStepParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterMoveColorTemperatureParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull moveMode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull rate MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull colorTemperatureMinimumMireds MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull colorTemperatureMaximumMireds MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRColorControlClusterStepColorTemperatureParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull stepMode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull stepSize MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull transitionTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull colorTemperatureMinimumMireds MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull colorTemperatureMaximumMireds MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsMask MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull optionsOverride MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRChannelClusterChangeChannelParams : NSObject <NSCopying>

@property (nonatomic, copy) NSString * _Nonnull match MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRChannelClusterChangeChannelResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable data MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRChannelClusterChangeChannelResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRChannelClusterChangeChannelByNumberParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull majorNumber MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull minorNumber MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRChannelClusterSkipChannelParams : NSObject <NSCopying>

@property (nonatomic, copy, getter=getCount) NSNumber * _Nonnull count MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRTargetNavigatorClusterNavigateTargetParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull target MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable data MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRTargetNavigatorClusterNavigateTargetResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable data MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRTargetNavigatorClusterNavigateTargetResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
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

MTR_DEPRECATED("Please use MTRMediaPlaybackClusterStopParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRMediaPlaybackClusterStopPlaybackParams : MTRMediaPlaybackClusterStopParams
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRMediaPlaybackClusterSkipForwardParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull deltaPositionMilliseconds MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRMediaPlaybackClusterSkipBackwardParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull deltaPositionMilliseconds MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRMediaPlaybackClusterPlaybackResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable data MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRMediaPlaybackClusterPlaybackResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRMediaPlaybackClusterSeekParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull position MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRMediaInputClusterSelectInputParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull index MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRMediaInputClusterRenameInputParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull index MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nonnull name MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRKeypadInputClusterSendKeyParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull keyCode MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRKeypadInputClusterSendKeyResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRKeypadInputClusterSendKeyResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRContentLauncherClusterLaunchContentParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRContentLauncherClusterContentSearchStruct * _Nonnull search MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull autoPlay MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable data MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRContentLauncherClusterLaunchURLParams : NSObject <NSCopying>

@property (nonatomic, copy) NSString * _Nonnull contentURL MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nullable displayString MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) MTRContentLauncherClusterBrandingInformationStruct * _Nullable brandingInformation MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRContentLauncherClusterLauncherResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSString * _Nullable data MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRContentLauncherClusterLauncherResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_DEPRECATED("Please use MTRContentLauncherClusterLauncherResponseParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRContentLauncherClusterLaunchResponseParams : MTRContentLauncherClusterLauncherResponseParams

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_DEPRECATED("The status field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSString * _Nullable data MTR_DEPRECATED("The data field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRAudioOutputClusterSelectOutputParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull index MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRAudioOutputClusterRenameOutputParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull index MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nonnull name MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRApplicationLauncherClusterLaunchAppParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRApplicationLauncherClusterApplicationStruct * _Nullable application MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSData * _Nullable data MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRApplicationLauncherClusterStopAppParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRApplicationLauncherClusterApplicationStruct * _Nullable application MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRApplicationLauncherClusterHideAppParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRApplicationLauncherClusterApplicationStruct * _Nullable application MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRApplicationLauncherClusterLauncherResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSData * _Nullable data MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRApplicationLauncherClusterLauncherResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRAccountLoginClusterGetSetupPINParams : NSObject <NSCopying>

@property (nonatomic, copy) NSString * _Nonnull tempAccountIdentifier MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRAccountLoginClusterGetSetupPINResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSString * _Nonnull setupPIN MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRAccountLoginClusterGetSetupPINResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRAccountLoginClusterLoginParams : NSObject <NSCopying>

@property (nonatomic, copy) NSString * _Nonnull tempAccountIdentifier MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSString * _Nonnull setupPIN MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRElectricalMeasurementClusterGetProfileInfoResponseCommandParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull profileCount MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull profileIntervalPeriod MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull maxNumberOfIntervals MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSArray * _Nonnull listOfAttributes MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRElectricalMeasurementClusterGetProfileInfoResponseCommandParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
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

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRElectricalMeasurementClusterGetMeasurementProfileResponseCommandParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull startTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull profileIntervalPeriod MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull numberOfIntervalsDelivered MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull attributeId MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSArray * _Nonnull intervals MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRElectricalMeasurementClusterGetMeasurementProfileResponseCommandParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRElectricalMeasurementClusterGetMeasurementProfileCommandParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull attributeId MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull startTime MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

@property (nonatomic, copy) NSNumber * _Nonnull numberOfIntervals MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
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

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestParams : MTRUnitTestingClusterTestParams
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestSpecificResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull returnValue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRUnitTestingClusterTestSpecificResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestSpecificResponseParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestSpecificResponseParams : MTRUnitTestingClusterTestSpecificResponseParams

@property (nonatomic, copy) NSNumber * _Nonnull returnValue MTR_DEPRECATED("The returnValue field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
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

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestNotHandledParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestNotHandledParams : MTRUnitTestingClusterTestNotHandledParams
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestAddArgumentsResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull returnValue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRUnitTestingClusterTestAddArgumentsResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestAddArgumentsResponseParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestAddArgumentsResponseParams : MTRUnitTestingClusterTestAddArgumentsResponseParams

@property (nonatomic, copy) NSNumber * _Nonnull returnValue MTR_DEPRECATED("The returnValue field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
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

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestSpecificParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestSpecificParams : MTRUnitTestingClusterTestSpecificParams
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestSimpleArgumentResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull returnValue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRUnitTestingClusterTestSimpleArgumentResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestSimpleArgumentResponseParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestSimpleArgumentResponseParams : MTRUnitTestingClusterTestSimpleArgumentResponseParams

@property (nonatomic, copy) NSNumber * _Nonnull returnValue MTR_DEPRECATED("The returnValue field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
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

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestUnknownCommandParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestUnknownCommandParams : MTRUnitTestingClusterTestUnknownCommandParams
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestStructArrayArgumentResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull arg1 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSArray * _Nonnull arg2 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSArray * _Nonnull arg3 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSArray * _Nonnull arg4 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull arg5 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull arg6 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRUnitTestingClusterTestStructArrayArgumentResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestStructArrayArgumentResponseParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestStructArrayArgumentResponseParams : MTRUnitTestingClusterTestStructArrayArgumentResponseParams

@property (nonatomic, copy) NSArray * _Nonnull arg1 MTR_DEPRECATED("The arg1 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSArray * _Nonnull arg2 MTR_DEPRECATED("The arg2 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSArray * _Nonnull arg3 MTR_DEPRECATED("The arg3 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSArray * _Nonnull arg4 MTR_DEPRECATED("The arg4 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull arg5 MTR_DEPRECATED("The arg5 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull arg6 MTR_DEPRECATED("The arg6 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestAddArgumentsParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull arg1 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull arg2 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestAddArgumentsParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestAddArgumentsParams : MTRUnitTestingClusterTestAddArgumentsParams

@property (nonatomic, copy) NSNumber * _Nonnull arg1 MTR_DEPRECATED("The arg1 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull arg2 MTR_DEPRECATED("The arg2 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestListInt8UReverseResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull arg1 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRUnitTestingClusterTestListInt8UReverseResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestListInt8UReverseResponseParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestListInt8UReverseResponseParams : MTRUnitTestingClusterTestListInt8UReverseResponseParams

@property (nonatomic, copy) NSArray * _Nonnull arg1 MTR_DEPRECATED("The arg1 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestSimpleArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull arg1 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestSimpleArgumentRequestParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestSimpleArgumentRequestParams : MTRUnitTestingClusterTestSimpleArgumentRequestParams

@property (nonatomic, copy) NSNumber * _Nonnull arg1 MTR_DEPRECATED("The arg1 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestEnumsResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull arg1 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull arg2 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRUnitTestingClusterTestEnumsResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestEnumsResponseParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestEnumsResponseParams : MTRUnitTestingClusterTestEnumsResponseParams

@property (nonatomic, copy) NSNumber * _Nonnull arg1 MTR_DEPRECATED("The arg1 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull arg2 MTR_DEPRECATED("The arg2 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestStructArrayArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull arg1 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSArray * _Nonnull arg2 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSArray * _Nonnull arg3 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSArray * _Nonnull arg4 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull arg5 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull arg6 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestStructArrayArgumentRequestParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestStructArrayArgumentRequestParams : MTRUnitTestingClusterTestStructArrayArgumentRequestParams

@property (nonatomic, copy) NSArray * _Nonnull arg1 MTR_DEPRECATED("The arg1 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSArray * _Nonnull arg2 MTR_DEPRECATED("The arg2 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSArray * _Nonnull arg3 MTR_DEPRECATED("The arg3 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSArray * _Nonnull arg4 MTR_DEPRECATED("The arg4 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull arg5 MTR_DEPRECATED("The arg5 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull arg6 MTR_DEPRECATED("The arg6 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestNullableOptionalResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull wasPresent MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable wasNull MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable value MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable originalValue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRUnitTestingClusterTestNullableOptionalResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestNullableOptionalResponseParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestNullableOptionalResponseParams : MTRUnitTestingClusterTestNullableOptionalResponseParams

@property (nonatomic, copy) NSNumber * _Nonnull wasPresent MTR_DEPRECATED("The wasPresent field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nullable wasNull MTR_DEPRECATED("The wasNull field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nullable value MTR_DEPRECATED("The value field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nullable originalValue MTR_DEPRECATED("The originalValue field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestStructArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nonnull arg1 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestStructArgumentRequestParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestStructArgumentRequestParams : MTRUnitTestingClusterTestStructArgumentRequestParams

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nonnull arg1 MTR_DEPRECATED("The arg1 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestComplexNullableOptionalResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull nullableIntWasNull MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable nullableIntValue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull optionalIntWasPresent MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable optionalIntValue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull nullableOptionalIntWasPresent MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalIntWasNull MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalIntValue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull nullableStringWasNull MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSString * _Nullable nullableStringValue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull optionalStringWasPresent MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSString * _Nullable optionalStringValue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull nullableOptionalStringWasPresent MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalStringWasNull MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSString * _Nullable nullableOptionalStringValue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull nullableStructWasNull MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable nullableStructValue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull optionalStructWasPresent MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable optionalStructValue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull nullableOptionalStructWasPresent MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalStructWasNull MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable nullableOptionalStructValue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull nullableListWasNull MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSArray * _Nullable nullableListValue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull optionalListWasPresent MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSArray * _Nullable optionalListValue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull nullableOptionalListWasPresent MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalListWasNull MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSArray * _Nullable nullableOptionalListValue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRUnitTestingClusterTestComplexNullableOptionalResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestComplexNullableOptionalResponseParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestComplexNullableOptionalResponseParams : MTRUnitTestingClusterTestComplexNullableOptionalResponseParams

@property (nonatomic, copy) NSNumber * _Nonnull nullableIntWasNull MTR_DEPRECATED("The nullableIntWasNull field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nullable nullableIntValue MTR_DEPRECATED("The nullableIntValue field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull optionalIntWasPresent MTR_DEPRECATED("The optionalIntWasPresent field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nullable optionalIntValue MTR_DEPRECATED("The optionalIntValue field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull nullableOptionalIntWasPresent MTR_DEPRECATED("The nullableOptionalIntWasPresent field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalIntWasNull MTR_DEPRECATED("The nullableOptionalIntWasNull field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalIntValue MTR_DEPRECATED("The nullableOptionalIntValue field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull nullableStringWasNull MTR_DEPRECATED("The nullableStringWasNull field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSString * _Nullable nullableStringValue MTR_DEPRECATED("The nullableStringValue field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull optionalStringWasPresent MTR_DEPRECATED("The optionalStringWasPresent field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSString * _Nullable optionalStringValue MTR_DEPRECATED("The optionalStringValue field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull nullableOptionalStringWasPresent MTR_DEPRECATED("The nullableOptionalStringWasPresent field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalStringWasNull MTR_DEPRECATED("The nullableOptionalStringWasNull field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSString * _Nullable nullableOptionalStringValue MTR_DEPRECATED("The nullableOptionalStringValue field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull nullableStructWasNull MTR_DEPRECATED("The nullableStructWasNull field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable nullableStructValue MTR_DEPRECATED("The nullableStructValue field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull optionalStructWasPresent MTR_DEPRECATED("The optionalStructWasPresent field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable optionalStructValue MTR_DEPRECATED("The optionalStructValue field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull nullableOptionalStructWasPresent MTR_DEPRECATED("The nullableOptionalStructWasPresent field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalStructWasNull MTR_DEPRECATED("The nullableOptionalStructWasNull field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable nullableOptionalStructValue MTR_DEPRECATED("The nullableOptionalStructValue field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull nullableListWasNull MTR_DEPRECATED("The nullableListWasNull field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSArray * _Nullable nullableListValue MTR_DEPRECATED("The nullableListValue field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull optionalListWasPresent MTR_DEPRECATED("The optionalListWasPresent field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSArray * _Nullable optionalListValue MTR_DEPRECATED("The optionalListValue field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull nullableOptionalListWasPresent MTR_DEPRECATED("The nullableOptionalListWasPresent field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalListWasNull MTR_DEPRECATED("The nullableOptionalListWasNull field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSArray * _Nullable nullableOptionalListValue MTR_DEPRECATED("The nullableOptionalListValue field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestNestedStructArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRUnitTestingClusterNestedStruct * _Nonnull arg1 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestNestedStructArgumentRequestParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestNestedStructArgumentRequestParams : MTRUnitTestingClusterTestNestedStructArgumentRequestParams

@property (nonatomic, copy) MTRUnitTestingClusterNestedStruct * _Nonnull arg1 MTR_DEPRECATED("The arg1 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterBooleanResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull value MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRUnitTestingClusterBooleanResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterBooleanResponseParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterBooleanResponseParams : MTRUnitTestingClusterBooleanResponseParams

@property (nonatomic, copy) NSNumber * _Nonnull value MTR_DEPRECATED("The value field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestListStructArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull arg1 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestListStructArgumentRequestParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestListStructArgumentRequestParams : MTRUnitTestingClusterTestListStructArgumentRequestParams

@property (nonatomic, copy) NSArray * _Nonnull arg1 MTR_DEPRECATED("The arg1 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterSimpleStructResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nonnull arg1 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRUnitTestingClusterSimpleStructResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterSimpleStructResponseParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterSimpleStructResponseParams : MTRUnitTestingClusterSimpleStructResponseParams

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nonnull arg1 MTR_DEPRECATED("The arg1 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestListInt8UArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull arg1 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestListInt8UArgumentRequestParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestListInt8UArgumentRequestParams : MTRUnitTestingClusterTestListInt8UArgumentRequestParams

@property (nonatomic, copy) NSArray * _Nonnull arg1 MTR_DEPRECATED("The arg1 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestEmitTestEventResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull value MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRUnitTestingClusterTestEmitTestEventResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestEmitTestEventResponseParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestEmitTestEventResponseParams : MTRUnitTestingClusterTestEmitTestEventResponseParams

@property (nonatomic, copy) NSNumber * _Nonnull value MTR_DEPRECATED("The value field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestNestedStructListArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRUnitTestingClusterNestedStructList * _Nonnull arg1 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestNestedStructListArgumentRequestParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestNestedStructListArgumentRequestParams : MTRUnitTestingClusterTestNestedStructListArgumentRequestParams

@property (nonatomic, copy) MTRUnitTestingClusterNestedStructList * _Nonnull arg1 MTR_DEPRECATED("The arg1 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestEmitTestFabricScopedEventResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull value MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRUnitTestingClusterTestEmitTestFabricScopedEventResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));
@end

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestEmitTestFabricScopedEventResponseParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestEmitTestFabricScopedEventResponseParams : MTRUnitTestingClusterTestEmitTestFabricScopedEventResponseParams

@property (nonatomic, copy) NSNumber * _Nonnull value MTR_DEPRECATED("The value field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestListNestedStructListArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull arg1 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestListNestedStructListArgumentRequestParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestListNestedStructListArgumentRequestParams : MTRUnitTestingClusterTestListNestedStructListArgumentRequestParams

@property (nonatomic, copy) NSArray * _Nonnull arg1 MTR_DEPRECATED("The arg1 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestListInt8UReverseRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSArray * _Nonnull arg1 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestListInt8UReverseRequestParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestListInt8UReverseRequestParams : MTRUnitTestingClusterTestListInt8UReverseRequestParams

@property (nonatomic, copy) NSArray * _Nonnull arg1 MTR_DEPRECATED("The arg1 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestEnumsRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull arg1 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull arg2 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestEnumsRequestParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestEnumsRequestParams : MTRUnitTestingClusterTestEnumsRequestParams

@property (nonatomic, copy) NSNumber * _Nonnull arg1 MTR_DEPRECATED("The arg1 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull arg2 MTR_DEPRECATED("The arg2 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestNullableOptionalRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nullable arg1 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestNullableOptionalRequestParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestNullableOptionalRequestParams : MTRUnitTestingClusterTestNullableOptionalRequestParams

@property (nonatomic, copy) NSNumber * _Nullable arg1 MTR_DEPRECATED("The arg1 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestComplexNullableOptionalRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nullable nullableInt MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable optionalInt MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalInt MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSString * _Nullable nullableString MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSString * _Nullable optionalString MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSString * _Nullable nullableOptionalString MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable nullableStruct MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable optionalStruct MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable nullableOptionalStruct MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSArray * _Nullable nullableList MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSArray * _Nullable optionalList MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSArray * _Nullable nullableOptionalList MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestComplexNullableOptionalRequestParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestComplexNullableOptionalRequestParams : MTRUnitTestingClusterTestComplexNullableOptionalRequestParams

@property (nonatomic, copy) NSNumber * _Nullable nullableInt MTR_DEPRECATED("The nullableInt field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nullable optionalInt MTR_DEPRECATED("The optionalInt field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nullable nullableOptionalInt MTR_DEPRECATED("The nullableOptionalInt field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSString * _Nullable nullableString MTR_DEPRECATED("The nullableString field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSString * _Nullable optionalString MTR_DEPRECATED("The optionalString field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSString * _Nullable nullableOptionalString MTR_DEPRECATED("The nullableOptionalString field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable nullableStruct MTR_DEPRECATED("The nullableStruct field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable optionalStruct MTR_DEPRECATED("The optionalStruct field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nullable nullableOptionalStruct MTR_DEPRECATED("The nullableOptionalStruct field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSArray * _Nullable nullableList MTR_DEPRECATED("The nullableList field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSArray * _Nullable optionalList MTR_DEPRECATED("The optionalList field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSArray * _Nullable nullableOptionalList MTR_DEPRECATED("The nullableOptionalList field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterSimpleStructEchoRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nonnull arg1 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_DEPRECATED("Please use MTRUnitTestingClusterSimpleStructEchoRequestParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterSimpleStructEchoRequestParams : MTRUnitTestingClusterSimpleStructEchoRequestParams

@property (nonatomic, copy) MTRUnitTestingClusterSimpleStruct * _Nonnull arg1 MTR_DEPRECATED("The arg1 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
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

MTR_DEPRECATED("Please use MTRUnitTestingClusterTimedInvokeRequestParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTimedInvokeRequestParams : MTRUnitTestingClusterTimedInvokeRequestParams
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestSimpleOptionalArgumentRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nullable arg1 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestSimpleOptionalArgumentRequestParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestSimpleOptionalArgumentRequestParams : MTRUnitTestingClusterTestSimpleOptionalArgumentRequestParams

@property (nonatomic, copy) NSNumber * _Nullable arg1 MTR_DEPRECATED("The arg1 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestEmitTestEventRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull arg1 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull arg2 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * _Nonnull arg3 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestEmitTestEventRequestParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestEmitTestEventRequestParams : MTRUnitTestingClusterTestEmitTestEventRequestParams

@property (nonatomic, copy) NSNumber * _Nonnull arg1 MTR_DEPRECATED("The arg1 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull arg2 MTR_DEPRECATED("The arg2 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * _Nonnull arg3 MTR_DEPRECATED("The arg3 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRUnitTestingClusterTestEmitTestFabricScopedEventRequestParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull arg1 MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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

MTR_DEPRECATED("Please use MTRUnitTestingClusterTestEmitTestFabricScopedEventRequestParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRTestClusterClusterTestEmitTestFabricScopedEventRequestParams : MTRUnitTestingClusterTestEmitTestFabricScopedEventRequestParams

@property (nonatomic, copy) NSNumber * _Nonnull arg1 MTR_DEPRECATED("The arg1 field will be removed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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
@interface MTRSampleMEIClusterPingParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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
@interface MTRSampleMEIClusterAddArgumentsResponseParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull returnValue MTR_PROVISIONALLY_AVAILABLE;

/**
 * Initialize an MTRSampleMEIClusterAddArgumentsResponseParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_PROVISIONALLY_AVAILABLE;
@end

MTR_PROVISIONALLY_AVAILABLE
@interface MTRSampleMEIClusterAddArgumentsParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull arg1 MTR_PROVISIONALLY_AVAILABLE;

@property (nonatomic, copy) NSNumber * _Nonnull arg2 MTR_PROVISIONALLY_AVAILABLE;
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
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
