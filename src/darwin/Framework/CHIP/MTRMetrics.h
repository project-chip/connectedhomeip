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
 * A representation of a collection of metrics data for an operation.
 */
MTR_NEWLY_AVAILABLE
@interface MTRMetrics : NSObject

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * @brief Returns the names of all the metrics data items collected.
 */
@property (nonatomic, readonly, copy) NSArray<NSString *> * allKeys;

/**
 * @brief Returns metric object corresponding to the metric identified by its key
 *
 * @param [in] key Name of the metric
 *
 * @return An object containing the metric data, nil if key is invalid
 */
- (nullable id)valueForKey:(NSString *)key;

@end

NS_ASSUME_NONNULL_END
