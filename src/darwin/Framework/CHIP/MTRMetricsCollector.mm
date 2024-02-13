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
#import "MTRMetricsCollector.h"
#include <MTRMetrics.h>
#import "MTRLogging_Internal.h"
#import <Matter/MTRDefines.h>
#include <platform/Darwin/Tracing.h>
#include <tracing/registry.h>

void InitializeMetricsCollection()
{
    if ([MTRMetricsCollector sharedInstance])
    {
        MTR_LOG_INFO("Initialized metrics collection backend for Darwin");
    }
}

@implementation MTRMetricsCollector {
    NSMutableDictionary<NSString *, id> * _metricsData;
    chip::Tracing::signposts::DarwinTracingBackend _tracingBackend;
}

+ (instancetype)sharedInstance
{
    static MTRMetricsCollector * singleton = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        // initialize the singleton.
        singleton = [[MTRMetricsCollector alloc] init];
        if (singleton) {
            chip::Tracing::Register(singleton->_tracingBackend);
        }
    });
    return singleton;
}

- (instancetype)init
{
    if (!(self = [super init])) {
        return nil;
    }
    return self;
}

@end
