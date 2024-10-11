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

#include <MTRDefines.h>
#import <Matter/MTRMetrics.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * This function initializes any backend required to collect metrics data.
 */
void StartupMetricsCollection();

/**
 * This function shuts down any backend created to collect metrics data.
 */
void ShutdownMetricsCollection();

/**
 * A representation of metrics data for an operation.
 */
@interface MTRMetricsCollector : NSObject

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * Return the singleton MTRMetricsCollector to vend MTRMetrics snapshots
 */
+ (instancetype)sharedInstance;

/**
 * @brief This method creates a snapshot of the metrics collected until the current point in time
 * and returns an object with the stats.
 *
 * @param [in] resetCollection Boolean that specifies whether or not to clear the stats collected after
 *                             creating the snapshot.
 *
 * @return MTRMetric object representing the metric data.
 */
- (MTRMetrics *)metricSnapshot:(BOOL)resetCollection;

/**
 * @brief This method clears any metrics collected.
 */
- (void)resetMetrics;

@end

NS_ASSUME_NONNULL_END
