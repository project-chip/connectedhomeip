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
#include <system/SystemClock.h>
#include <tracing/metric_event.h>
#include <tracing/registry.h>

using MetricEvent = chip::Tracing::MetricEvent;

static NSString * kMTRMetricDataValueKey = @"value";
static NSString * kMTRMetricDataTimepointKey = @"time_point";
static NSString * kMTRMetricDataDurationKey = @"duration_us";

@implementation MTRMetricsData {
    chip::System::Clock::Microseconds64 _timePoint;
    chip::System::Clock::Microseconds64 _duration;
}

- (instancetype)initWithMetricEvent:(const MetricEvent &)event
{
    if (!(self = [super init])) {
        return nil;
    }

    using ValueType = MetricEvent::Value::Type;
    switch (event.ValueType()) {
    case ValueType::kInt32:
        _value = [NSNumber numberWithInteger:event.ValueInt32()];
        break;
    case ValueType::kUInt32:
        _value = [NSNumber numberWithInteger:event.ValueUInt32()];
        break;
    case ValueType::kChipErrorCode:
        _value = [NSNumber numberWithInteger:event.ValueErrorCode()];
        break;
    case ValueType::kUndefined:
    default:
        _value = nil;
    }

    _timePoint = chip::System::SystemClock().GetMonotonicMicroseconds64();
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
    return [NSString stringWithFormat:@"MTRMetricsData: Value = %@, TimePoint = %@, Duration = %@ us", self.value, self.timePointMicroseconds, self.durationMicroseconds];
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

@interface MTRMetricsCollector ()

- (void)registerTracingBackend;

- (void)unregisterTracingBackend;

@end

void StartupMetricsCollection()
{
    if ([MTRMetricsCollector sharedInstance]) {
        MTR_LOG_INFO("Initialized metrics collection backend for Darwin");

        [[MTRMetricsCollector sharedInstance] registerTracingBackend];
    }
}

void ShutdownMetricsCollection()
{
    [[MTRMetricsCollector sharedInstance] unregisterTracingBackend];
}

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
        // Initialize the singleton and register the event handler
        singleton = [[MTRMetricsCollector alloc] init];
        if (singleton) {
            singleton->_tracingBackend.SetMetricEventHandler(^(MetricEvent event) {
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

- (void)registerTracingBackend
{
    std::lock_guard lock(_lock);
    chip::Tracing::Register(_tracingBackend);
    MTR_LOG_INFO("Registered tracing backend with the registry");
}

- (void)unregisterTracingBackend
{
    std::lock_guard lock(_lock);
    chip::Tracing::Unregister(_tracingBackend);
    MTR_LOG_INFO("Unregistered tracing backend with the registry");
}

static inline NSString * suffixNameForMetricType(MetricEvent::Type type)
{
    switch (type) {
    case MetricEvent::Type::kBeginEvent:
        return @"-begin";
    case MetricEvent::Type::kEndEvent:
        return @"-end";
    case MetricEvent::Type::kInstantEvent:
        return @"-instant";
    }
}

static inline NSString * suffixNameForMetric(const MetricEvent & event)
{
    return suffixNameForMetricType(event.type());
}

- (void)handleMetricEvent:(MetricEvent)event
{
    std::lock_guard lock(_lock);

    using ValueType = MetricEvent::Value::Type;
    switch (event.ValueType()) {
    case ValueType::kInt32:
        MTR_LOG_INFO("Received metric event, key: %s, type: %d, value: %d", event.key(), event.type(), event.ValueInt32());
        break;
    case ValueType::kUInt32:
        MTR_LOG_INFO("Received metric event, key: %s, type: %d, value: %u", event.key(), event.type(), event.ValueUInt32());
        break;
    case ValueType::kChipErrorCode:
        MTR_LOG_INFO("Received metric event, key: %s, type: %d, error value: %u", event.key(), event.type(), event.ValueErrorCode());
        break;
    case ValueType::kUndefined:
        MTR_LOG_INFO("Received metric event, key: %s, type: %d, value: nil", event.key(), event.type());
        break;
    default:
        MTR_LOG_INFO("Received metric event, key: %s, type: %d, unknown value", event.key(), event.type());
        return;
    }

    // Create the new metric key based event type
    auto metricsKey = [NSString stringWithFormat:@"%s%@", event.key(), suffixNameForMetric(event)];
    MTRMetricsData * data = [[MTRMetricsData alloc] initWithMetricEvent:event];

    // If End event, compute its duration using the Begin event
    if (event.type() == MetricEvent::Type::kEndEvent) {
        auto metricsBeginKey = [NSString stringWithFormat:@"%s%@", event.key(), suffixNameForMetricType(MetricEvent::Type::kBeginEvent)];
        MTRMetricsData * beginMetric = _metricsDataCollection[metricsBeginKey];
        if (beginMetric) {
            [data setDurationFromMetricData:beginMetric];
        } else {
            // Unbalanced end
            MTR_LOG_ERROR("Unable to find Begin event corresponding to Metric Event: %s", event.key());
        }
    }

    [_metricsDataCollection setValue:data forKey:metricsKey];

    // If the event is a begin or end event, implicitly emit a corresponding instant event
    if (event.type() == MetricEvent::Type::kBeginEvent || event.type() == MetricEvent::Type::kEndEvent) {
        MetricEvent instantEvent(MetricEvent::Type::kInstantEvent, event.key());
        data = [[MTRMetricsData alloc] initWithMetricEvent:instantEvent];
        metricsKey = [NSString stringWithFormat:@"%s%@", event.key(), suffixNameForMetric(instantEvent)];
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
