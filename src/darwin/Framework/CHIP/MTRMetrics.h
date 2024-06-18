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

#import <Foundation/Foundation.h>
#import <Matter/MTRDefines.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * Representation of metric data corresponding to a metric event.
 */
MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRMetricData : NSObject

/**
 * Value for the metric data. The value may be nil depending on the event emitted.
 */
@property (nonatomic, nullable, readonly, copy) NSNumber * value;

/**
 * Error code for the metric data. This value, when not nil, holds the error code value
 * of the operation associated with the event. Interpretation of the error code value
 * dependents on the metric being emitted.
 */
@property (nonatomic, nullable, readonly, copy) NSNumber * errorCode;

/**
 * Duration of event associated with the metric. This value may be nil depending on
 * the event emitted. When not nil, the value of duration is of type NSTimeInterval.
 */
@property (nonatomic, nullable, readonly, copy) NSNumber * duration;

@end

/**
 * A representation of a collection of metrics data for an operation.
 */
MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRMetrics : NSObject

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * @brief Returns a unique identifier for the object
 */
@property (nonatomic, readonly, copy) NSUUID * uniqueIdentifier;

/**
 * @brief Returns the names of all the metrics data items collected.
 */
@property (nonatomic, readonly, copy) NSArray<NSString *> * allKeys;

/**
 * @brief Returns metric data corresponding to the metric identified by its key.
 *
 * @param [in] key Name of the metric
 *
 * @return An object containing the metric data, nil if key is invalid.
 */
- (nullable MTRMetricData *)metricDataForKey:(NSString *)key;

@end

NS_ASSUME_NONNULL_END
