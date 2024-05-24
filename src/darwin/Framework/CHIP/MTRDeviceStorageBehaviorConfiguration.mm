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

#import "MTRDeviceStorageBehaviorConfiguration.h"

#import "MTRLogging_Internal.h"

#define kReportToPersistenceDelayTimeDefault (15)
#define kReportToPersistenceDelayTimeMaxDefault (20 * kReportToPersistenceDelayTimeDefault)
#define kRecentReportTimesMaxCountDefault (12)
#define kTimeBetweenReportsTooShortThresholdDefault (15)
#define kTimeBetweenReportsTooShortMinThresholdDefault (5)
#define kReportToPersistenceDelayMaxMultiplierDefault (10)
#define kDeviceReportingExcessivelyIntervalThresholdDefault (5 * 60)

@implementation MTRDeviceStorageBehaviorConfiguration

+ (instancetype)configurationWithReportToPersistenceDelayTime:(NSTimeInterval)reportToPersistenceDelayTime
                              reportToPersistenceDelayTimeMax:(NSTimeInterval)reportToPersistenceDelayTimeMax
                                    recentReportTimesMaxCount:(NSUInteger)recentReportTimesMaxCount
                          timeBetweenReportsTooShortThreshold:(NSTimeInterval)timeBetweenReportsTooShortThreshold
                       timeBetweenReportsTooShortMinThreshold:(NSTimeInterval)timeBetweenReportsTooShortMinThreshold
                        reportToPersistenceDelayMaxMultiplier:(double)reportToPersistenceDelayMaxMultiplier
                  deviceReportingExcessivelyIntervalThreshold:(NSTimeInterval)deviceReportingExcessivelyIntervalThreshold
{
    auto newConfiguration = [[MTRDeviceStorageBehaviorConfiguration alloc] init];
    newConfiguration.reportToPersistenceDelayTime = reportToPersistenceDelayTime;
    newConfiguration.reportToPersistenceDelayTimeMax = reportToPersistenceDelayTimeMax;
    newConfiguration.recentReportTimesMaxCount = recentReportTimesMaxCount;
    newConfiguration.timeBetweenReportsTooShortThreshold = timeBetweenReportsTooShortThreshold;
    newConfiguration.timeBetweenReportsTooShortMinThreshold = timeBetweenReportsTooShortMinThreshold;
    newConfiguration.reportToPersistenceDelayMaxMultiplier = reportToPersistenceDelayMaxMultiplier;
    newConfiguration.deviceReportingExcessivelyIntervalThreshold = deviceReportingExcessivelyIntervalThreshold;

    return newConfiguration;
}

+ (instancetype)configurationWithDefaultStorageBehavior
{
    auto newConfiguration = [[MTRDeviceStorageBehaviorConfiguration alloc] init];
    [newConfiguration checkValuesAndResetToDefaultIfNecessary];
    return newConfiguration;
}

+ (instancetype)configurationWithStorageBehaviorOptimizationDisabled
{
    auto newConfiguration = [[MTRDeviceStorageBehaviorConfiguration alloc] init];
    newConfiguration.disableStorageBehaviorOptimization = YES;
    return newConfiguration;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<MTRDeviceStorageBehaviorConfiguration(%p): disabled %s reportToPersistenceDelayTime %lf reportToPersistenceDelayTimeMax %lf recentReportTimesMaxCount %lu timeBetweenReportsTooShortThreshold %lf timeBetweenReportsTooShortMinThreshold %lf reportToPersistenceDelayMaxMultiplier %lf deviceReportingExcessivelyIntervalThreshold %lf", self, _disableStorageBehaviorOptimization ? "YES" : "NO", _reportToPersistenceDelayTime, _reportToPersistenceDelayTimeMax, static_cast<unsigned long>(_recentReportTimesMaxCount), _timeBetweenReportsTooShortThreshold, _timeBetweenReportsTooShortMinThreshold, _reportToPersistenceDelayMaxMultiplier, _deviceReportingExcessivelyIntervalThreshold];
}

- (void)checkValuesAndResetToDefaultIfNecessary
{
    if (_disableStorageBehaviorOptimization) {
        return;
    }

    // Sanity check all the values, and if any is out of range, reset to default values
    if ((_reportToPersistenceDelayTime <= 0) || (_reportToPersistenceDelayTimeMax <= 0) || (_reportToPersistenceDelayTimeMax < _reportToPersistenceDelayTime) || (_recentReportTimesMaxCount < 2) || (_timeBetweenReportsTooShortThreshold <= 0) || (_timeBetweenReportsTooShortMinThreshold <= 0) || (_timeBetweenReportsTooShortMinThreshold > _timeBetweenReportsTooShortThreshold) || (_reportToPersistenceDelayMaxMultiplier <= 1) || (_deviceReportingExcessivelyIntervalThreshold <= 0)) {
        MTR_LOG_ERROR("%@ storage behavior: MTRDeviceStorageBehaviorConfiguration values out of bounds - resetting to default", self);

        _reportToPersistenceDelayTime = kReportToPersistenceDelayTimeDefault;
        _reportToPersistenceDelayTimeMax = kReportToPersistenceDelayTimeMaxDefault;
        _recentReportTimesMaxCount = kRecentReportTimesMaxCountDefault;
        _timeBetweenReportsTooShortThreshold = kTimeBetweenReportsTooShortThresholdDefault;
        _timeBetweenReportsTooShortMinThreshold = kTimeBetweenReportsTooShortMinThresholdDefault;
        _reportToPersistenceDelayMaxMultiplier = kReportToPersistenceDelayMaxMultiplierDefault;
        _deviceReportingExcessivelyIntervalThreshold = kDeviceReportingExcessivelyIntervalThresholdDefault;
    }
}

- (id)copyWithZone:(NSZone *)zone
{
    auto newConfiguration = [[MTRDeviceStorageBehaviorConfiguration alloc] init];
    newConfiguration.disableStorageBehaviorOptimization = _disableStorageBehaviorOptimization;
    newConfiguration.reportToPersistenceDelayTime = _reportToPersistenceDelayTime;
    newConfiguration.reportToPersistenceDelayTimeMax = _reportToPersistenceDelayTimeMax;
    newConfiguration.recentReportTimesMaxCount = _recentReportTimesMaxCount;
    newConfiguration.timeBetweenReportsTooShortThreshold = _timeBetweenReportsTooShortThreshold;
    newConfiguration.timeBetweenReportsTooShortMinThreshold = _timeBetweenReportsTooShortMinThreshold;
    newConfiguration.reportToPersistenceDelayMaxMultiplier = _reportToPersistenceDelayMaxMultiplier;
    newConfiguration.deviceReportingExcessivelyIntervalThreshold = _deviceReportingExcessivelyIntervalThreshold;

    return newConfiguration;
}

@end
