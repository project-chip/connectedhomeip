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
#import "MTRMetrics.h"
#import "MTRMetrics_Internal.h"
#import <MTRUnfairLock.h>
#import <os/lock.h>
#include <platform/Darwin/Tracing.h>
#include <system/SystemClock.h>
#include <tracing/metric_event.h>
#include <tracing/registry.h>

/*
 * Set this to MTR_LOG_DEBUG(__VA_ARGS__) to enable logging noisy debug logging for metrics events processing
 */
#define MTR_METRICS_LOG_DEBUG(...)

using MetricEvent = chip::Tracing::MetricEvent;

@implementation MTRMetricData {
    chip::System::Clock::Microseconds64 _timePoint;
    MetricEvent::Type _type;
}

- (instancetype)init
{
    // Default is to create data for instant event type.
    // The key can be anything since it is not really used in this context.
    MetricEvent event(MetricEvent::Type::kInstantEvent, "");
    return [self initWithMetricEvent:event];
}

- (instancetype)initWithMetricEvent:(const MetricEvent &)event
{
    if (!(self = [super init])) {
        return nil;
    }

    _type = event.type();

    using EventType = MetricEvent::Type;
    switch (_type) {
    // Capture timepoint for begin and end to calculate duration
    case EventType::kBeginEvent:
    case EventType::kEndEvent:
        _timePoint = chip::System::SystemClock().GetMonotonicMicroseconds64();
        break;
    case EventType::kInstantEvent:
        _timePoint = chip::System::Clock::Microseconds64(0);
        break;
    }

    using ValueType = MetricEvent::Value::Type;
    switch (event.ValueType()) {
    case ValueType::kInt32:
        _value = [NSNumber numberWithInteger:event.ValueInt32()];
        break;
    case ValueType::kUInt32:
        _value = [NSNumber numberWithUnsignedInteger:event.ValueUInt32()];
        break;
    case ValueType::kChipErrorCode:
        _errorCode = [NSNumber numberWithUnsignedInteger:event.ValueErrorCode()];
        break;
    case ValueType::kUndefined:
        break;
    }

    MTR_METRICS_LOG_DEBUG("Initializing metric event data %s, type: %d, with time point %llu", event.key(), _type, _timePoint.count());
    return self;
}

- (void)setDurationFromMetricData:(MTRMetricData *)fromData
{
    auto duration = _timePoint - fromData->_timePoint;
    _duration = [NSNumber numberWithDouble:double(duration.count()) / USEC_PER_SEC];

    MTR_METRICS_LOG_DEBUG("Calculating duration for Matter metric with type %d, from type %d, (%llu - %llu) = %llu us (%llu s)",
        _type, fromData->_type, _timePoint.count(), fromData->_timePoint.count(), duration.count(), [_duration unsignedLongLongValue]);
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<MTRMetricData: Type %d, Value = %@, Error Code = %@, Duration = %@ us>",
                     static_cast<int>(_type), self.value, self.errorCode, self.duration];
}

@end

@interface MTRMetricsCollector ()

- (void)registerTracingBackend;

- (void)unregisterTracingBackend;

@end

void StartupMetricsCollection()
{
    if ([MTRMetricsCollector sharedInstance]) {
        MTR_LOG("Initialized metrics collection backend for Darwin");

        [[MTRMetricsCollector sharedInstance] registerTracingBackend];
    }
}

void ShutdownMetricsCollection()
{
    [[MTRMetricsCollector sharedInstance] unregisterTracingBackend];
}

@implementation MTRMetricsCollector {
    os_unfair_lock _lock;
    NSMutableDictionary<NSString *, MTRMetricData *> * _metricsDataCollection;
    chip::Tracing::signposts::DarwinTracingBackend _tracingBackend;
    BOOL _tracingBackendRegistered;
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
    _tracingBackendRegistered = FALSE;
    return self;
}

- (void)registerTracingBackend
{
    std::lock_guard lock(_lock);

    // Register only once
    if (!_tracingBackendRegistered) {
        chip::Tracing::Register(_tracingBackend);
        MTR_LOG("Registered tracing backend with the registry");
        _tracingBackendRegistered = TRUE;
    }
}

- (void)unregisterTracingBackend
{
    std::lock_guard lock(_lock);

    // Unregister only if registered before
    if (_tracingBackendRegistered) {
        chip::Tracing::Unregister(_tracingBackend);
        MTR_LOG("Unregistered tracing backend with the registry");
        _tracingBackendRegistered = FALSE;
    }
}

static inline NSString * suffixNameForMetricType(MetricEvent::Type type)
{
    switch (type) {
    case MetricEvent::Type::kBeginEvent:
        return @"_begin";
    case MetricEvent::Type::kEndEvent:
        return @"_end";
    case MetricEvent::Type::kInstantEvent:
        return @"_event";
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
        MTR_METRICS_LOG_DEBUG("Received metric event, key: %s, type: %d, value: %d", event.key(), static_cast<int>(event.type()), event.ValueInt32());
        break;
    case ValueType::kUInt32:
        MTR_METRICS_LOG_DEBUG("Received metric event, key: %s, type: %d, value: %u", event.key(), static_cast<int>(event.type()), event.ValueUInt32());
        break;
    case ValueType::kChipErrorCode:
        MTR_METRICS_LOG_DEBUG("Received metric event, key: %s, type: %d, error value: %u", event.key(), static_cast<int>(event.type()), event.ValueErrorCode());
        break;
    case ValueType::kUndefined:
        MTR_METRICS_LOG_DEBUG("Received metric event, key: %s, type: %d, value: nil", event.key(), static_cast<int>(event.type()));
        break;
    default:
        MTR_METRICS_LOG_DEBUG("Received metric event, key: %s, type: %d, unknown value", event.key(), static_cast<int>(event.type()));
        return;
    }

    // Create the new metric key based event type
    auto metricsKey = [NSString stringWithFormat:@"%s%@", event.key(), suffixNameForMetric(event)];
    MTRMetricData * data = [[MTRMetricData alloc] initWithMetricEvent:event];

    // If End event, compute its duration using the Begin event
    if (event.type() == MetricEvent::Type::kEndEvent) {
        auto metricsBeginKey = [NSString stringWithFormat:@"%s%@", event.key(), suffixNameForMetricType(MetricEvent::Type::kBeginEvent)];
        MTRMetricData * beginMetric = _metricsDataCollection[metricsBeginKey];
        if (beginMetric) {
            [data setDurationFromMetricData:beginMetric];
        } else {
            // Unbalanced end
            MTR_LOG_ERROR("Unable to find Begin event corresponding to Metric Event: %s", event.key());
        }
    }

    // Add to the collection only if it does not exist as yet.
    if (![_metricsDataCollection valueForKey:metricsKey]) {
        [_metricsDataCollection setValue:data forKey:metricsKey];
    }
}

- (MTRMetrics *)metricSnapshot:(BOOL)resetCollection
{
    std::lock_guard lock(_lock);

    MTRMetrics * metrics = [[MTRMetrics alloc] initWithCapacity:[_metricsDataCollection count]];
    for (NSString * key in _metricsDataCollection) {
        [metrics setMetricData:_metricsDataCollection[key] forKey:key];
    }

    // Clear curent stats, if specified
    if (resetCollection) {
        [_metricsDataCollection removeAllObjects];
    }
    return metrics;
}

- (void)resetMetrics
{
    std::lock_guard lock(_lock);
    [_metricsDataCollection removeAllObjects];
}

@end
