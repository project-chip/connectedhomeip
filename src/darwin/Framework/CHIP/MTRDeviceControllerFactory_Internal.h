/**
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

/**
 * Parts of MTRDeviceControllerFactory that are not part of the framework API.
 * Mostly for use from MTRDeviceController.
 */

#import <Foundation/Foundation.h>
#import <Matter/MTRAccessGrant.h>
#import <Matter/MTRBaseDevice.h> // for MTRClusterPath
#import <Matter/MTRDefines.h>
#import <Matter/MTRDeviceController.h>
#import <Matter/MTRDeviceControllerParameters.h>
#import <Matter/MTRDiagnosticLogsType.h>
#import <Matter/MTRServerEndpoint.h>

#import "MTRDefines_Internal.h"
#import "MTRDeviceControllerFactory.h"

#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/PeerId.h>

namespace chip {
namespace Credentials {
    class GroupDataProvider;
} // namespace Credentials
} // namespace chip

@class MTRDeviceController;

NS_ASSUME_NONNULL_BEGIN

MTR_DIRECT_MEMBERS
@interface MTRDeviceControllerFactory ()

- (void)controllerShuttingDown:(MTRDeviceController *)controller;

/**
 * Get the list of running controllers.  This will include controllers that are
 * in the middle of starting up or shutting down.
 */
- (NSArray<MTRDeviceController *> *)getRunningControllers;

/**
 * Find a running controller, if any, for the given fabric index.
 */
- (nullable MTRDeviceController *)runningControllerForFabricIndex:(chip::FabricIndex)fabricIndex;

/**
 * Find a running controller, if any, for the given fabric index.  Allows
 * controlling whether to include a controller that is in the middle of startup
 * or shutdown.
 */
- (nullable MTRDeviceController *)runningControllerForFabricIndex:(chip::FabricIndex)fabricIndex
                                      includeControllerStartingUp:(BOOL)includeControllerStartingUp
                                    includeControllerShuttingDown:(BOOL)includeControllerShuttingDown;

/**
 * Notify the controller factory that a new operational instance with the given
 * compressed fabric id and node id has been observed.
 */
- (void)operationalInstanceAdded:(chip::PeerId &)operationalID;

/**
 * Download log of the desired type from the device.
 */
- (void)downloadLogFromNodeWithID:(NSNumber *)nodeID
                       controller:(MTRDeviceController *)controller
                             type:(MTRDiagnosticLogType)type
                          timeout:(NSTimeInterval)timeout
                            queue:(dispatch_queue_t)queue
                       completion:(void (^)(NSURL * _Nullable url, NSError * _Nullable error))completion;

/**
 * Initialize an MTRDeviceController with the given parameters.
 */
- (nullable MTRDeviceController *)initializeController:(MTRDeviceController *)controller
                                        withParameters:(MTRDeviceControllerParameters *)parameters
                                                 error:(NSError * __autoreleasing *)error;

/**
 * Add a server endpoint.  This will verify that there is no existing server
 * endpoint with the provided endpoint ID and return NO if there is one.  Can be
 * called on any thread.
 */
- (BOOL)addServerEndpoint:(MTRServerEndpoint *)endpoint;

/**
 * Remove a server endpoint.  This must happen after all other teardown for the
 * endpoint is complete.  Can be called on any thread.
 */
- (void)removeServerEndpoint:(MTRServerEndpoint *)endpoint;

@property (readonly) chip::PersistentStorageDelegate * storageDelegate;
@property (readonly) chip::Credentials::GroupDataProvider * groupDataProvider;

@end

MTR_DIRECT_MEMBERS
@interface MTRDeviceControllerFactoryParams ()
/*
 * Initialize the device controller factory without storage.  In this mode,
 * device controllers will need to have per-controller storage provided to allow
 * storing controller-specific information.
 */
- (instancetype)initWithoutStorage;
@end

// Methods accessed from MTRServerAccessControl linked into darwin-framework-tool
// TODO: https://github.com/project-chip/connectedhomeip/issues/32991
@interface MTRDeviceControllerFactory ()

/**
 * Get the access grants that apply for the given fabric index and cluster path.
 *
 * Only called on the Matter queue.
 */
- (NSArray<MTRAccessGrant *> *)accessGrantsForFabricIndex:(chip::FabricIndex)fabricIndex clusterPath:(MTRClusterPath *)clusterPath;

/**
 * Get the privilege level needed to read the given attribute.  There's no
 * endpoint provided because the expectation is that this information is the
 * same for all cluster instances.
 *
 * Returns nil if we have no such attribute defined on any endpoint, otherwise
 * one of MTRAccessControlEntry* constants wrapped in NSNumber.
 *
 * Only called on the Matter queue.
 */
- (nullable NSNumber *)neededReadPrivilegeForClusterID:(NSNumber *)clusterID attributeID:(NSNumber *)attributeID;

@end

NS_ASSUME_NONNULL_END
