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
#include <tracing/scalar_event.h>
#include <MTRDefines_Internal.h>
#include <MTRMetrics_Internal.h>
#import "MTRLogging_Internal.h"
#import <Matter/MTRDefines.h>
#include <platform/Darwin/Tracing.h>
#include <tracing/registry.h>
#import "MTRUnfairLock.h"

void InitializeMetricsCollection()
{
    if ([MTRMetricsCollector sharedInstance])
    {
        MTR_LOG_INFO("Initialized metrics collection backend for Darwin");
    }
}

@implementation MTRMetricsCollector {
    os_unfair_lock _lock;
    NSMutableDictionary<NSString *, NSNumber*> * _metricsData;
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
            singleton->_tracingBackend.SetLogEventClientCallback(^(chip::Tracing::ScalarEvent event) {
                    MTR_LOG_INFO("RECEIVED scalar event, type: %u, value: %u", event.eventType, event.eventValue);
                    if (singleton) {
                        [singleton handleScalarEvent:event];
                    }
                });
        }
    });
    return singleton;
}

- (instancetype)init
{
    if (!(self = [super init])) {
        return nil;
    }
    _lock = OS_UNFAIR_LOCK_INIT;
    _metricsData = [NSMutableDictionary dictionary];
    return self;
}

static NSString * convertEventTypeToString(chip::Tracing::ScalarEvent::EventType eventType)
{
    using chip::Tracing::ScalarEvent;
    switch (eventType) {
        case ScalarEvent::kDiscoveryOverBLE: return @"com.matter.metric.disovered-over-ble";
        case ScalarEvent::kDiscoveryOnNetwork: return @"com.matter.metric.disovered-on-network";
        case ScalarEvent::kPASEConnectionEstablished: return @"com.matter.metric.pase-connection-established";
        case ScalarEvent::kPASEConnectionFailed: return @"com.matter.metric.pase-connection-failed";
        case ScalarEvent::kAttestationResult: return @"com.matter.metric.attestation-result";
        case ScalarEvent::kAttestationOverridden: return @"com.matter.metric.attestation-overridden";
        case ScalarEvent::kCASEConnectionEstablished: return @"com.matter.metric.case-connection-established";
        case ScalarEvent::kCASEConnectionFailed: return @"com.matter.metric.case-connection-failed";
    }
}

- (void)handleScalarEvent:(chip::Tracing::ScalarEvent)event
{
    MTR_LOG_INFO("Received scalar event, type: %u, value: %u", event.eventType, event.eventValue);
    std::lock_guard lock(_lock);
    [_metricsData setValue:[NSNumber numberWithUnsignedInteger:event.eventValue]
                    forKey:convertEventTypeToString(event.eventType)];
}

- (MTRMetrics *)metricSnapshot:(BOOL)resetCollection
{
    std::lock_guard lock(_lock);
    MTRMetrics *metrics = [MTRMetrics metricsFromDictionary:_metricsData];
    if (resetCollection) {
       [_metricsData removeAllObjects];
    }
    return metrics;
}

@end
