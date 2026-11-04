/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#import "MTRMetrics.h"
#include <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface MTRMetrics ()

- (instancetype)initWithCapacity:(NSUInteger)numItems;

- (void)setMetricData:(MTRMetricData * _Nullable)value forKey:(NSString *)key;

- (void)removeMetricDataForKey:(NSString *)key;

// The flow ID for a UUID string received from another process, or a new flow ID if it is not one.
+ (NSUUID *)_flowIDWithString:(id _Nullable)flowID;

// The current flow ID, or a new one if there is none.
+ (NSUUID *)_currentOrNewFlowID;

// Runs block with flowID current on this thread, then restores the previous current flow ID.
+ (void)_runWithFlowID:(NSUUID * _Nullable)flowID block:(void(NS_NOESCAPE ^)(void))block;

@end

NS_ASSUME_NONNULL_END
