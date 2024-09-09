/**
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
 * Class that configures how MTRDevice objects persist their attributes to storage, so as to not
 * overwhelm the underlying storage system.
 */
MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRDeviceStorageBehaviorConfiguration : NSObject <NSCopying>

/**
 * Create configuration with a default set of values. See description below for details.
 */
+ (instancetype)configurationWithDefaultStorageBehavior;

/**
 * Create configuration that disables storage behavior optimizations.
 */
+ (instancetype)configurationWithStorageBehaviorOptimizationDisabled;

/**
 * Create configuration with specified values. See description below for details, and the list of
 * properties below for valid ranges of these values.
 */
+ (instancetype)configurationWithReportToPersistenceDelayTime:(NSTimeInterval)reportToPersistenceDelayTime
                              reportToPersistenceDelayTimeMax:(NSTimeInterval)reportToPersistenceDelayTimeMax
                                    recentReportTimesMaxCount:(NSUInteger)recentReportTimesMaxCount
                          timeBetweenReportsTooShortThreshold:(NSTimeInterval)timeBetweenReportsTooShortThreshold
                       timeBetweenReportsTooShortMinThreshold:(NSTimeInterval)timeBetweenReportsTooShortMinThreshold
                        reportToPersistenceDelayMaxMultiplier:(double)reportToPersistenceDelayMaxMultiplier
                  deviceReportingExcessivelyIntervalThreshold:(NSTimeInterval)deviceReportingExcessivelyIntervalThreshold;

/**
 * Storage behavior with values in the allowed range:
 *
 * Each time a report comes in, MTRDevice will wait reportToPersistDelayTime before persisting the
 * changes to storage. If another report comes in during this internal, MTRDevice will wait another
 * reportToPersistDelayTime interval, until reportToPersistDelayTimeMax is reached, at which
 * point all the changes so far will be written to storage.
 *
 * MTRDevice will also track recentReportTimesMaxCount number of report times. If the running
 * average time between reports dips below timeBetweenReportsTooShortThreshold, a portion of the
 * reportToPersistenceDelayMaxMultiplier will be applied to both the reportToPersistenceDelayTime
 * and reportToPersistenceDelayTimeMax. The multiplier will reach the max when the average time
 * between reports reach timeBetweenReportsTooShortMinThreshold.
 *
 * When the running average time between reports dips below timeBetweenReportsTooShortMinThreshold
 * for the first time, the time will be noted. If the device remains in this state for longer than
 * deviceReportingExcessivelyIntervalThreshold, persistence will stop until the average time between
 * reports go back above timeBetweenReportsTooShortMinThreshold.
 */

/**
 * If disableStorageBehaviorOptimization is set to YES, then all the waiting mechanism as described above
 * is disabled.
 */
@property (nonatomic, assign) BOOL disableStorageBehaviorOptimization;

/**
 * If any of these properties are set to be out of the documented limits, these default values will
 * be used to replace all of them:
 *
 * reportToPersistenceDelayTimeDefault (15)
 * reportToPersistenceDelayTimeMaxDefault (20 * 15)
 * recentReportTimesMaxCountDefault (12)
 * timeBetweenReportsTooShortThresholdDefault (15)
 * timeBetweenReportsTooShortMinThresholdDefault (5)
 * reportToPersistenceDelayMaxMultiplierDefault (10)
 * deviceReportingExcessivelyIntervalThresholdDefault (5 * 60)
 */
@property (nonatomic, assign) NSTimeInterval reportToPersistenceDelayTime; /* must be > 0 */
@property (nonatomic, assign) NSTimeInterval reportToPersistenceDelayTimeMax; /* must be larger than reportToPersistenceDelayTime */
@property (nonatomic, assign) NSUInteger recentReportTimesMaxCount; /* must be >= 2 */
@property (nonatomic, assign) NSTimeInterval timeBetweenReportsTooShortThreshold; /* must be > 0 */
@property (nonatomic, assign) NSTimeInterval timeBetweenReportsTooShortMinThreshold; /* must be > 0 and smaller than timeBetweenReportsTooShortThreshold */
@property (nonatomic, assign) double reportToPersistenceDelayMaxMultiplier; /* must be > 1 */
@property (nonatomic, assign) NSTimeInterval deviceReportingExcessivelyIntervalThreshold; /* must be > 0 */
@end

NS_ASSUME_NONNULL_END
