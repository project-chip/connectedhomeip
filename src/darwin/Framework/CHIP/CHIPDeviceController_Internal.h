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
 * for use from MatterStack.
 */

#import <Foundation/Foundation.h>

#import "CHIPDeviceController.h"

#include <lib/core/DataModelTypes.h>

@class CHIPDeviceControllerStartupParams;
@class MatterStack;

NS_ASSUME_NONNULL_BEGIN

@interface CHIPDeviceController (InternalMethods)

/**
 * Start a new controller.  Returns whether startup succeeded.  If this fails,
 * it guarantees that it has called controllerShuttingDown on the MatterStack.
 *
 * Only MatterStack should be calling this.
 */
- (BOOL)startup:(CHIPDeviceControllerStartupParams *)startupParams;

/*
 * Will return chip::kUndefinedFabricIndex if we do not have a fabric index.
 */
@property (readonly) chip::FabricIndex fabricIndex;

/*
 * Init a newly created controller.
 *
 * Only MatterStack should be calling this.
 */
- (instancetype)initWithStack:(MatterStack *)stack queue:(dispatch_queue_t)queue;

@end

NS_ASSUME_NONNULL_END

#endif // MATTER_DEVICE_CONTROLLER_INTERNAL_H
