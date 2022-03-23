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

#import "CHIPDeviceControllerOverXPC.h"

NS_ASSUME_NONNULL_BEGIN

@class CHIPSubscribeParams;

@interface CHIPDeviceControllerOverXPC (AttributeCache)

- (void)subscribeAttributeCacheWithNodeId:(uint64_t)nodeId
                                   params:(CHIPSubscribeParams * _Nullable)params
                               completion:(void (^)(NSError * _Nullable error))completion;

- (void)readAttributeCacheWithNodeId:(uint64_t)nodeId
                          endpointId:(NSNumber * _Nullable)endpointId
                           clusterId:(NSNumber * _Nullable)clusterId
                         attributeId:(NSNumber * _Nullable)attributeId
                          completion:(void (^)(id _Nullable values, NSError * _Nullable error))completion;

@end

NS_ASSUME_NONNULL_END
