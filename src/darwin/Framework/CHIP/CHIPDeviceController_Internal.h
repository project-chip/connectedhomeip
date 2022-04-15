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

#ifndef MATTER_DEVICE_CONTROLLER_INTERNAL_H
#define MATTER_DEVICE_CONTROLLER_INTERNAL_H

/**
 * Parts of CHIPDeviceController that are not part of the framework API.  Mostly
 * for use from MatterControllerFactory.
 */

#import <Foundation/Foundation.h>

#import "CHIPDeviceController.h"

#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

@class CHIPDeviceControllerStartupParams;
@class MatterControllerFactory;

namespace chip {
class FabricInfo;
} // namespace chip

NS_ASSUME_NONNULL_BEGIN

@interface CHIPDeviceController (InternalMethods)

/**
 * Start a new controller.  Returns whether startup succeeded.  If this fails,
 * it guarantees that it has called controllerShuttingDown on the
 * MatterControllerFactory.
 *
 * The return value will always match [controller isRunning] for this
 * controller.
 *
 * Only MatterControllerFactory should be calling this.
 */
// TODO: This needs to be refactored to allow specifying things like the NOC to
// use (instead of always generating a new one).
- (BOOL)startup:(CHIPDeviceControllerStartupParams *)startupParams;

/**
 * Will return chip::kUndefinedFabricIndex if we do not have a fabric index.
 * This property MUST be gotten from the Matter work queue.
 */
@property (readonly) chip::FabricIndex fabricIndex;

/**
 * Init a newly created controller.
 *
 * Only MatterControllerFactory should be calling this.
 */
- (instancetype)initWithFactory:(MatterControllerFactory *)factory queue:(dispatch_queue_t)queue;

/**
 * Check whether this controller is running on the given fabric.  The FabricInfo
 * might be from a one-off fabric table.  This method MUST be called from the
 * Matter work queue.
 *
 * Might return failure, in which case we don't know whether it's running on the
 * given fabric.  Otherwise it will set *isRunning to the right boolean value.
 *
 * Only MatterControllerFactory should be calling this.
 */
- (CHIP_ERROR)isRunningOnFabric:(chip::FabricInfo *)fabric isRunning:(BOOL *)isRunning;

@end

NS_ASSUME_NONNULL_END

#endif // MATTER_DEVICE_CONTROLLER_INTERNAL_H
