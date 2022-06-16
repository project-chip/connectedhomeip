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

#ifndef MTR_CONTROLLER_FACTORY_INTERNAL_H
#define MTR_CONTROLLER_FACTORY_INTERNAL_H

/**
 * Parts of MTRControllerFactory that are not part of the framework API.
 * Mostly for use from CHIPDeviceController.
 */

#import <Foundation/Foundation.h>

#import "MTRControllerFactory.h"

class MTRPersistentStorageDelegateBridge;

namespace chip {
namespace Credentials {
    class GroupDataProvider;
} // namespace Credentials
} // namespace chip

@class MTRDeviceController;

NS_ASSUME_NONNULL_BEGIN

@interface MTRControllerFactory (InternalMethods)

- (void)controllerShuttingDown:(MTRDeviceController *)controller;

@property (readonly) MTRPersistentStorageDelegateBridge * storageDelegateBridge;
@property (readonly) chip::Credentials::GroupDataProvider * groupData;

@end

NS_ASSUME_NONNULL_END

#endif // MTR_CONTROLLER_FACTORY_INTERNAL_H
