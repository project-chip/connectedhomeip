/**
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

#import <Foundation/Foundation.h>

#import "MTRDevice.h"

#include <app/DeviceProxy.h>

NS_ASSUME_NONNULL_BEGIN

typedef void (^MTRDevicePerformAsyncBlock)(chip::DeviceProxy * internalDevice);

@interface MTRDevice ()
- (instancetype)initWithDeviceID:(uint64_t)deviceID
                deviceController:(MTRDeviceController *)deviceController
                           queue:(dispatch_queue_t)queue;

// Called from within the MTRAsyncCallbackReadyHandler of a MTRAsyncCallbackQueueWorkItem
- (void)connectAndPerformAsync:(MTRDevicePerformAsyncBlock)asyncBlock;

// Called from MTRClusters for writes and commands
- (void)setExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)values
    expectedValueInterval:(NSNumber *)expectedValueIntervalMs;

@property (nonatomic, readonly, strong, nonnull) MTRDeviceController * deviceController;
@property (nonatomic, readonly) uint64_t deviceID;
@property (nonatomic, readonly, strong, nonnull) MTRAsyncCallbackWorkQueue * asyncCallbackQueue;

@end

NS_ASSUME_NONNULL_END
