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
#include <sys/_types/_key_t.h>
#include <ctime>
#include <chrono>
#include <cmath>
#include <MTRMetrics.h>
#include <tracing/metric_event.h>
#include <MTRDefines_Internal.h>
#include <MTRMetrics_Internal.h>
#import "MTRLogging_Internal.h"
#import <Matter/MTRDefines.h>
#include <platform/Darwin/Tracing.h>
#include <tracing/registry.h>
#import "MTRUnfairLock.h"

using MetricEvent = chip::Tracing::MetricEvent;

void InitializeMetricsCollection()
{
    if ([MTRMetricsCollector sharedInstance])
    {
        MTR_LOG_INFO("Initialized metrics collection backend for Darwin");
    }
}

@implementation MTRMetricsData {
    std::chrono::steady_clock::time_point timePoint;
    std::chrono::steady_clock::duration duration;
}

- (instancetype)initWithMetricEvent:(const MetricEvent&) event
{
    if (!(self = [super init])) {
        return nil;
    }

    if (event.value.type == MetricEvent::Value::ValueType::SignedValue) {
        _value = [NSNumber numberWithInteger:event.value.store.svalue];
    }
    else {
        _value = [NSNumber numberWithUnsignedInteger:event.value.store.uvalue];
    }
    timePoint = event.timePoint;
    duration = std::chrono::steady_clock::duration();
    return self;
}

- (void)setDurationFromMetricData:(MTRMetricsData *)fromData
{
    duration = timePoint - fromData->timePoint;
}

- (NSNumber *)timePointNanoseconds
{
    auto ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(timePoint);
    return [NSNumber numberWithLongLong:ns.time_since_epoch().count()];
}

- (NSNumber *)durationNanoseconds
{
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
    return [NSNumber numberWithLongLong:ns.count()];
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"Value = %@, TimePoint = %@, Duration = %@", self.value, self.timePointNanoseconds, self.durationNanoseconds];
}

@end

@implementation MTRMetricsCollector {
    os_unfair_lock _lock;
    NSMutableDictionary<NSString *, MTRMetricsData*> * _metricsDataCollection;
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
            singleton->_tracingBackend.SetLogEventClientCallback(^(MetricEvent event) {
                    if (singleton) {
                        [singleton handleMetricEvent:event];
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
    _metricsDataCollection = [NSMutableDictionary dictionary];
    return self;
}

static NSString * suffixNameForMetricTag(MetricEvent::Tag tag)
{
    switch (tag) {
        case MetricEvent::Tag::Begin: return @"-begin";
        case MetricEvent::Tag::End: return @"-end";
        case MetricEvent::Tag::Instant: return @"-instant";
    }
}

static inline NSString * suffixNameForMetricTag(const MetricEvent & event)
{
    return suffixNameForMetricTag(event.tag);
}

- (void)handleMetricEvent:(MetricEvent)event
{
    MTR_LOG_INFO("Received metric event, type: %s, value: %u", event.key, event.value.store.uvalue);
    std::lock_guard lock(_lock);

    auto metricsKey = [NSString stringWithFormat:@"%s%@", event.key, suffixNameForMetricTag(event)];
    MTRMetricsData *data = [[MTRMetricsData alloc] initWithMetricEvent:event];

    // If End event, compute its duration using the Begin event
    if (event.tag == MetricEvent::Tag::End) {
        auto metricsBeginKey = [NSString stringWithFormat:@"%s%@", event.key, suffixNameForMetricTag(MetricEvent::Tag::Begin)];
        auto beginMetric = _metricsDataCollection[metricsBeginKey];
        if (beginMetric) {
            [data setDurationFromMetricData:beginMetric];
        }
        else {
            MTR_LOG_ERROR("Unable to find Begin event corresponding to Metric Event: %s", event.key);
        }
    }

    [_metricsDataCollection setValue:data forKey:metricsKey];

    // If the event is a being or end event, implicitly emit a corresponding instant event
    if (event.tag == MetricEvent::Tag::Begin || event.tag == MetricEvent::Tag::End) {
        MetricEvent instantEvent(event);
        instantEvent.tag = MetricEvent::Tag::Instant;
        data = [[MTRMetricsData alloc] initWithMetricEvent:event];
        metricsKey = [NSString stringWithFormat:@"%s%@", event.key, suffixNameForMetricTag(MetricEvent::Tag::Instant)];
        [_metricsDataCollection setValue:data forKey:metricsKey];
    }
}

- (MTRMetrics *)metricSnapshot:(BOOL)resetCollection
{
    std::lock_guard lock(_lock);
    MTRMetrics *metrics = [MTRMetrics metricsFromDictionary:_metricsDataCollection ];
    if (resetCollection) {
       [_metricsDataCollection  removeAllObjects];
    }
    return metrics;
}

@end
