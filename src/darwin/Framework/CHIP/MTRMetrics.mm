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
#include <MTRMetrics.h>
#include <Foundation/Foundation.h>
#import "MTRMetrics_Internal.h"
#import <Matter/MTRDefines.h>

@implementation MTRMetrics {
    NSMutableDictionary<NSString *, MTRMetricsData *> * _metricsData;
}

- (instancetype)init
{
    if (self = [super init]) {
        _metricsData = [NSMutableDictionary dictionary];
    }
    return self;
}

- (instancetype)initWithDictionary:(NSDictionary<NSString *, MTRMetricsData *> *)metricsData
{
    if (self = [super init]) {
        _metricsData = [NSMutableDictionary dictionary];
        if (_metricsData) {
            [_metricsData addEntriesFromDictionary:metricsData];
        }
    }
    return self;
}


+ (instancetype)metricsFromDictionary:(NSDictionary<NSString *, MTRMetricsData *> *)metricsData
{
    MTRMetrics *metrics = [[MTRMetrics alloc] initWithDictionary:metricsData];
    return metrics;
}

- (NSArray<NSString *> *)allKeys
{
    return [_metricsData allKeys];
}

- (nullable MTRMetricsData *)valueForKey:(NSString *)key
{
    if (!key) {
        MTR_LOG_ERROR("Cannot get metrics value for nil key");
        return nil;
    }

    return _metricsData[key];
}

- (void)setValue:(MTRMetricsData * _Nullable)value forKey:(NSString *)key
{
    if (!key) {
        MTR_LOG_ERROR("Cannot set metrics value for nil key");
        return;
    }

    [_metricsData setValue:value forKey:key];
}

- (void)removeValueForKey:(NSString *)key
{
    if (!key) {
        MTR_LOG_ERROR("Cannot remove metrics value for nil key");
        return;
    }

    [_metricsData removeObjectForKey:key];
}

- (NSString *)description
{
    return [_metricsData description];
}

@end
