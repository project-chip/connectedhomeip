/**
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
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
#import <Matter/MTRDevice.h>

#import "MTRDeviceClusterData.h"
#import "MTRDeviceController_Concrete.h"

NS_ASSUME_NONNULL_BEGIN

@interface MTRDevice_Concrete : MTRDevice

- (instancetype)initWithNodeID:(NSNumber *)nodeID controller:(MTRDeviceController_Concrete *)controller;

// Called by controller when a new operational advertisement for what we think
// is this device's identity has been observed.  This could have
// false-positives, for example due to compressed fabric id collisions.
- (void)nodeMayBeAdvertisingOperational;

// Method to insert persisted cluster data
//   Contains data version information and attribute values.
- (void)setPersistedClusterData:(NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *)clusterData;

// Method to insert persisted data that pertains to the whole device.
- (void)setPersistedDeviceData:(NSDictionary<NSString *, id> *)data;

// Returns whether this MTRDevice_Concrete uses Thread for communication
- (BOOL)deviceUsesThread;

// For use from MTRDeviceController_Concrete when setting up a device instance.
- (void)setStorageBehaviorConfiguration:(MTRDeviceStorageBehaviorConfiguration *)storageBehaviorConfiguration;

#ifdef DEBUG
- (NSUInteger)unitTestAttributeCount;
#endif

@end

NS_ASSUME_NONNULL_END
