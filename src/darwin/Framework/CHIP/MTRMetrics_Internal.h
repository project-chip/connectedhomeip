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

/**
 * A representation of a metric data for an operation.
 */
@interface MTRMetricsData : NSObject

// Value for the metric. This can be null if the metric is just a fire event with no value
@property (nonatomic, nullable, readonly, copy) NSNumber * value;

// Relative time point at which the metric was emitted. This may be null.
@property (nonatomic, nullable, readonly, copy) NSNumber * timePointMicroseconds;

// During for the event. This may be null.
@property (nonatomic, nullable, readonly, copy) NSNumber * durationMicroseconds;

// Convert contents to a dictionary
- (NSDictionary *)toDictionary;

@end

@interface MTRMetrics ()

- (instancetype)initWithCapacity:(NSUInteger)numItems;

- (void)setValue:(id _Nullable)value forKey:(NSString *)key;

- (void)removeValueForKey:(NSString *)key;

@end

NS_ASSUME_NONNULL_END
