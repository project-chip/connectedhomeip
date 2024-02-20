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
#import "MTRLogging_Internal.h"
#include "MTRMetrics_Internal.h"
#import <MTRUnfairLock.h>
#include <platform/Darwin/Tracing.h>
#include <tracing/metric_event.h>
#include <tracing/registry.h>

using MetricEvent = chip::Tracing::MetricEvent;

static NSString * kMTRMetricDataValueKey = @"value";
static NSString * kMTRMetricDataTimepointKey = @"time_point";
static NSString * kMTRMetricDataDurationKey = @"duration_us";

void InitializeMetricsCollection()
{
    if ([MTRMetricsCollector sharedInstance]) {
        MTR_LOG_INFO("Initialized metrics collection backend for Darwin");
    }
}

@implementation MTRMetricsData {
    chip::System::Clock::Microseconds64 _timePoint;
    chip::System::Clock::Microseconds64 _duration;
}

- (instancetype)initWithMetricEvent:(const MetricEvent &)event
{
    if (!(self = [super init])) {
        return nil;
    }

    if (event.value.type == MetricEvent::Value::Type::Signed32Type) {
        _value = [NSNumber numberWithInteger:event.value.store.int32_value];
    } else {
        _value = [NSNumber numberWithUnsignedInteger:event.value.store.uint32_value];
    }
    _timePoint = event.timePoint;
    _duration = chip::System::Clock::Microseconds64(0);
    return self;
}

- (void)setDurationFromMetricData:(MTRMetricsData *)fromData
{
    _duration = _timePoint - fromData->_timePoint;
}

- (NSNumber *)timePointMicroseconds
{
    return [NSNumber numberWithUnsignedLongLong:_timePoint.count()];
}

- (NSNumber *)durationMicroseconds
{
    return [NSNumber numberWithUnsignedLongLong:_duration.count()];
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"Value = %@, TimePoint = %@, Duration = %@ us", self.value, self.timePointMicroseconds, self.durationMicroseconds];
}

- (NSDictionary *)toDictionary
{
    NSMutableDictionary * dictRepresentation = [NSMutableDictionary dictionary];
    if (self.value) {
        [dictRepresentation setValue:self.value forKey:kMTRMetricDataValueKey];
    }
    if (auto tmPt = self.timePointMicroseconds) {
        [dictRepresentation setValue:tmPt forKey:kMTRMetricDataTimepointKey];
    }
    if (auto duration = self.durationMicroseconds) {
        [dictRepresentation setValue:duration forKey:kMTRMetricDataDurationKey];
    }
    return dictRepresentation;
}

@end

@implementation MTRMetricsCollector {
    os_unfair_lock _lock;
    NSMutableDictionary<NSString *, MTRMetricsData *> * _metricsDataCollection;
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

static inline NSString * suffixNameForMetricTag(MetricEvent::Tag tag)
{
    switch (tag) {
    case MetricEvent::Tag::Begin:
        return @"-begin";
    case MetricEvent::Tag::End:
        return @"-end";
    case MetricEvent::Tag::Instant:
        return @"-instant";
    }
}

static inline NSString * suffixNameForMetricTag(const MetricEvent & event)
{
    return suffixNameForMetricTag(event.tag);
}

- (void)handleMetricEvent:(MetricEvent)event
{
    MTR_LOG_INFO("Received metric event, key: %s, type: %hhu value: %d",
        event.key,
        event.value.type,
        (event.value.type == MetricEvent::Value::Type::Signed32Type) ? event.value.store.int32_value : event.value.store.uint32_value);

    std::lock_guard lock(_lock);

    // Create the new metric key based event type
    auto metricsKey = [NSString stringWithFormat:@"%s%@", event.key, suffixNameForMetricTag(event)];
    MTRMetricsData * data = [[MTRMetricsData alloc] initWithMetricEvent:event];

    // If End event, compute its duration using the Begin event
    if (event.tag == MetricEvent::Tag::End) {
        auto metricsBeginKey = [NSString stringWithFormat:@"%s%@", event.key, suffixNameForMetricTag(MetricEvent::Tag::Begin)];
        MTRMetricsData * beginMetric = _metricsDataCollection[metricsBeginKey];
        if (beginMetric) {
            [data setDurationFromMetricData:beginMetric];
        } else {
            // Unbalanced end
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

    // Copy the MTRMetrics as NSDictionary
    MTRMetrics * metrics = [[MTRMetrics alloc] initWithCapacity:[_metricsDataCollection count]];
    for (NSString * key in _metricsDataCollection) {
        [metrics setValue:[_metricsDataCollection[key] toDictionary] forKey:key];
    }

    // Clear curent stats, if specified
    if (resetCollection) {
        [_metricsDataCollection removeAllObjects];
    }
    return metrics;
}

@end
