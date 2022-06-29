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

#import "MTRDevice.h"
#import "MTRDeviceControllerXPCConnection.h"

NS_ASSUME_NONNULL_BEGIN

@interface MTRDeviceOverXPC : MTRDevice

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
- (void)subscribeWithQueue:(dispatch_queue_t)queue
                minInterval:(uint16_t)minInterval
                maxInterval:(uint16_t)maxInterval
              reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
    subscriptionEstablished:(void (^_Nullable)(void))subscriptionEstablishedHandler NS_UNAVAILABLE;

- (instancetype)initWithController:(id<NSCopying>)controller
                          deviceId:(uint64_t)deviceId
                     xpcConnection:(MTRDeviceControllerXPCConnection *)xpcConnection;

@end

NS_ASSUME_NONNULL_END
