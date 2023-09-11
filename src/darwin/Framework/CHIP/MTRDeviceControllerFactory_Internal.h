/**
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * Parts of MTRDeviceControllerFactory that are not part of the framework API.
 * Mostly for use from MTRDeviceController.
 */

#import <Foundation/Foundation.h>
#import <Matter/MTRDefines.h>
#import <Matter/MTRDeviceController.h>

#if MTR_PER_CONTROLLER_STORAGE_ENABLED
#import <Matter/MTRDeviceControllerParameters.h>
#else
#import "MTRDeviceControllerParameters_Wrapper.h"
#endif // MTR_PER_CONTROLLER_STORAGE_ENABLED

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

@interface MTRDeviceControllerFactory (InternalMethods)

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
 * Initialize an MTRDeviceController with the given parameters.
 */
- (nullable MTRDeviceController *)initializeController:(MTRDeviceController *)controller
                                        withParameters:(MTRDeviceControllerParameters *)parameters
                                                 error:(NSError * __autoreleasing *)error;

@property (readonly) chip::PersistentStorageDelegate * storageDelegate;
@property (readonly) chip::Credentials::GroupDataProvider * groupData;

@end

@interface MTRDeviceControllerFactoryParams ()
/*
 * Initialize the device controller factory without storage.  In this mode,
 * device controllers will need to have per-controller storage provided to allow
 * storing controller-specific information.
 */
- (instancetype)initWithoutStorage;
@end

NS_ASSUME_NONNULL_END
