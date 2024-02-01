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
#import <Matter/MTRDefines.h>

@implementation MTRMetrics {
    NSMutableDictionary<NSString *, id> * _metricsData;
}

- (instancetype)init
{
    if (self = [super init]) {
        _metricsData = [NSMutableDictionary dictionary];
    }
    return self;
}

- (NSArray<NSString *> *)allKeys
{
    return [_metricsData allKeys];
}

- (nullable id)valueForKey:(NSString *)key
{
    if (!key) {
        MTR_LOG_ERROR("Cannot get metrics value for nil key");
        return nil;
    }

    return _metricsData[key];
}

- (void)setValue:(id _Nullable)value forKey:(NSString *)key
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
