/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * Parts of MTRControllerFactory that are not part of the framework API.
 * Mostly for use from MTRDeviceController.
 */

#import <Foundation/Foundation.h>

#import "MTRControllerFactory.h"

#include <lib/core/DataModelTypes.h>

class MTRPersistentStorageDelegateBridge;

namespace chip {
namespace Credentials {
    class GroupDataProvider;
    class DeviceAttestationVerifier;
} // namespace Credentials
} // namespace chip

@class MTRDeviceController;

NS_ASSUME_NONNULL_BEGIN

@interface MTRControllerFactory (InternalMethods)

- (void)controllerShuttingDown:(MTRDeviceController *)controller;

/**
 * Find a running controller, if any, for the given fabric index.
 */
- (nullable MTRDeviceController *)runningControllerForFabricIndex:(chip::FabricIndex)fabricIndex;

@property (readonly) MTRPersistentStorageDelegateBridge * storageDelegateBridge;
@property (readonly) chip::Credentials::GroupDataProvider * groupData;
@property (readonly) chip::Credentials::DeviceAttestationVerifier * deviceAttestationVerifier;
@end

NS_ASSUME_NONNULL_END
