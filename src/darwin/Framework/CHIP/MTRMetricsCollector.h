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

#include <lib/core/CHIPError.h>
#include <tracing/metric_event.h>

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

/**
 * Creates a collector that is not connected to the tracing backend, so it only holds what it is given through handleMetricEvent:,
 * beginMetricForKey: and endMetricIfBegunForKey:error:.  Only the sharedInstance is registered as a tracing backend (by
 * StartupMetricsCollection), so creating or releasing other instances does not affect that registration.
 */
- (instancetype)init;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * Return the singleton MTRMetricsCollector to vend MTRMetrics snapshots
 */
+ (instancetype)sharedInstance;

/**
 * @brief This method creates a snapshot of the metrics for commissioning collected until the current
 *        point in time and returns an object with the stats.
 *
 * @param [in] resetCollection Boolean that specifies whether or not to clear the stats collected after
 *                             creating the snapshot.
 *
 * @return MTRMetric object representing the metric data.
 */
- (MTRMetrics *)metricSnapshotForCommissioning:(BOOL)resetCollection;

/**
 * @brief This method creates a snapshot of the metrics for a category (other than for commissioning)
 *        that were collected until the current point in time and returns an object with those stats.
 *
 * @param [in] removeMetrics Boolean that specifies whether to clear the stats for the category after
 *                           creating the snapshot.
 *
 * @return MTRMetric object representing the metric data.
 */
- (MTRMetrics *)metricSnapshotForCategory:(NSString *)category removeMetrics:(BOOL)removeMetrics;

/**
 * Records a metric event.
 */
- (void)handleMetricEvent:(chip::Tracing::MetricEvent)event;

/**
 * Starts the begin/end metric for the key.  Whether a metric is open is inferred from the begin and end entries for the key.
 * If the metric is already open (a begin with no end), nothing changes and this returns NO.  Otherwise any earlier begin and end
 * entries for the key are discarded, a new begin is recorded, and this returns YES.
 */
- (BOOL)beginMetricForKey:(chip::Tracing::MetricKey)key;

/**
 * Ends the begin/end metric for the key with the given error.  If the metric is not open, nothing changes and this returns NO.
 */
- (BOOL)endMetricIfBegunForKey:(chip::Tracing::MetricKey)key error:(CHIP_ERROR)error;

/**
 * Returns the metrics collected, and removes the finished ones.  A begin that has not ended is included in the result but stays
 * collected, so that it can still end later.
 */
- (MTRMetrics *)takeFinishedMetrics;

/**
 * Returns the error code the begin/end metric for the key ended with, or nil if it has not ended.
 */
- (nullable NSNumber *)endErrorCodeForKey:(chip::Tracing::MetricKey)key;

/**
 * @brief This method clears any metrics collected.
 */
- (void)resetMetrics;

@end

NS_ASSUME_NONNULL_END
