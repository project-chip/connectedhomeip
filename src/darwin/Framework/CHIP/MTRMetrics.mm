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
#import "MTRLogging_Internal.h"
#import "MTRMetrics_Internal.h"
#include <Foundation/Foundation.h>
#import <Matter/MTRDefines.h>
#include <Matter/MTRMetrics.h>

@implementation MTRMetrics {
    NSMutableDictionary<NSString *, MTRMetricData *> * _metricsData;
}

- (instancetype)init
{
    NSAssert(false, @"'init' unavailable, use initWithCapacity: instead");
    return nil;
}

- (instancetype)initWithCapacity:(NSUInteger)numItems
{
    if (self = [super init]) {
        _metricsData = [NSMutableDictionary dictionaryWithCapacity:numItems];
        _uniqueIdentifier = [NSUUID UUID];
    }
    return self;
}

- (NSArray<NSString *> *)allKeys
{
    return [_metricsData allKeys];
}

- (nullable MTRMetricData *)metricDataForKey:(NSString *)key
{
    if (!key) {
        MTR_LOG_ERROR("Cannot get metrics value for nil key");
        return nil;
    }

    return _metricsData[key];
}

- (void)setMetricData:(MTRMetricData * _Nullable)value forKey:(NSString *)key
{
    if (!key) {
        MTR_LOG_ERROR("Cannot set metrics value for nil key");
        return;
    }

    [_metricsData setValue:value forKey:key];
}

- (void)removeMetricDataForKey:(NSString *)key
{
    if (!key) {
        MTR_LOG_ERROR("Cannot remove metrics value for nil key");
        return;
    }

    [_metricsData removeObjectForKey:key];
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<MTRMetrics: uuid = %@, data = %@>", _uniqueIdentifier, [_metricsData description]];
}

@end
