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

#import "MTRDeviceControllerFactory.h"

#include <lib/core/DataModelTypes.h>
#include <lib/core/PeerId.h>

class MTRPersistentStorageDelegateBridge;

namespace chip {
namespace Credentials {
    class GroupDataProvider;
    class DeviceAttestationVerifier;
} // namespace Credentials
} // namespace chip

@class MTRDeviceController;

NS_ASSUME_NONNULL_BEGIN

@interface MTRDeviceControllerFactory (InternalMethods)

- (void)controllerShuttingDown:(MTRDeviceController *)controller;

/**
 * Find a running controller, if any, for the given fabric index.
 */
- (nullable MTRDeviceController *)runningControllerForFabricIndex:(chip::FabricIndex)fabricIndex;

/**
 * Notify the controller factory that a new operational instance with the given
 * compressed fabric id and node id has been observed.
 */
- (void)operationalInstanceAdded:(chip::PeerId &)operationalID;

@property (readonly) MTRPersistentStorageDelegateBridge * storageDelegateBridge;
@property (readonly) chip::Credentials::GroupDataProvider * groupData;
@property (readonly) chip::Credentials::DeviceAttestationVerifier * deviceAttestationVerifier;
@end

NS_ASSUME_NONNULL_END
