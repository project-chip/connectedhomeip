/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#import <Matter/MTRBaseDevice.h>
#import <Matter/MTRCluster.h>
#import <Matter/MTRDevice.h>

#import "MTRBaseDevice_Internal.h"

#include <app/ReadPrepareParams.h>
#include <lib/core/DataModelTypes.h>

NS_ASSUME_NONNULL_BEGIN

@interface MTRCluster ()
@property (readonly, nonatomic) dispatch_queue_t callbackQueue;

- (instancetype)initWithEndpointID:(NSNumber *)endpointID queue:(dispatch_queue_t)queue;
@end

@interface MTRGenericBaseCluster ()
- (instancetype)initWithDevice:(MTRBaseDevice *)device
                    endpointID:(NSNumber *)endpointID
                         queue:(dispatch_queue_t)queue;

@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRGenericCluster ()
- (instancetype)initWithDevice:(MTRDevice *)device
                    endpointID:(NSNumber *)endpointID
                         queue:(dispatch_queue_t)queue;
@end

@interface MTRReadParams ()
/**
 * Copy state from this MTRReadParams to the ReadPreparaParams.
 */
- (void)toReadPrepareParams:(chip::app::ReadPrepareParams &)readPrepareParams;
@end

@interface MTRSubscribeParams ()
/**
 * Copy state from this MTRReadParams to the ReadPreparaParams.
 */
- (void)toReadPrepareParams:(chip::app::ReadPrepareParams &)readPrepareParams;
@end

NS_ASSUME_NONNULL_END
