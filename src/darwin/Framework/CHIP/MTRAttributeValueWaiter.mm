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

#import <Foundation/Foundation.h>
#import <os/lock.h>

#import <Matter/MTRError.h>

#import "MTRAttributeValueWaiter_Internal.h"
#import "MTRDevice_Internal.h"
#import "MTRError_Internal.h"
#import "MTRLogging_Internal.h"
#import "MTRUnfairLock.h"

@implementation MTRAwaitedAttributeState
- (instancetype)initWithValue:(MTRDeviceDataValueDictionary)value
{
    if (self = [super init]) {
        _valueSatisfied = NO;
        _value = value;
    }

    return self;
}
@end

MTR_DIRECT_MEMBERS
@interface MTRAttributeValueWaiter ()
@property (nonatomic, retain) NSDictionary<MTRAttributePath *, MTRAwaitedAttributeState *> * valueExpectations;
// Protected by the MTRDevice's lock.
@property (nonatomic, readwrite, retain) dispatch_queue_t queue;
@property (nonatomic, readwrite, copy, nullable) MTRStatusCompletion completion;
@property (nonatomic, retain, readwrite, nullable) dispatch_source_t expirationTimer;
@property (nonatomic, readonly, retain) MTRDevice * device;
@end

@implementation MTRAttributeValueWaiter {
    // Protects queue/completion and expirationTimer.
    os_unfair_lock _lock;
}

- (instancetype)initWithDevice:(MTRDevice *)device values:(NSDictionary<MTRAttributePath *, MTRDeviceDataValueDictionary> *)values queue:(dispatch_queue_t)queue completion:(MTRStatusCompletion)completion
{
    if (self = [super init]) {
        auto * valueExpectations = [NSMutableDictionary dictionaryWithCapacity:values.count];
        for (MTRAttributePath * path in values) {
            auto * valueExpectation = [[MTRAwaitedAttributeState alloc] initWithValue:values[path]];
            valueExpectations[path] = valueExpectation;
        }
        _valueExpectations = valueExpectations;
        _queue = queue;
        _completion = completion;
        _device = device;
        _UUID = [NSUUID UUID];
        _lock = OS_UNFAIR_LOCK_INIT;
    }

    return self;
}

- (void)dealloc
{
    [self cancel];
}

- (void)cancel
{
    [self.device _forgetAttributeWaiter:self];
    [self _notifyCancellation];
}

- (void)_notifyCancellation
{
    [self _notifyWithError:[MTRError errorForCHIPErrorCode:CHIP_ERROR_CANCELLED]];
}

- (BOOL)_attributeValue:(MTRDeviceDataValueDictionary)value reportedForPath:(MTRAttributePath *)path byDevice:(MTRDevice *)device
{
    MTRAwaitedAttributeState * valueExpectation = self.valueExpectations[path];
    if (!valueExpectation) {
        // We don't care about this one.
        return NO;
    }

    MTRDeviceDataValueDictionary expectedValue = valueExpectation.value;
    valueExpectation.valueSatisfied = [device _attributeDataValue:value satisfiesValueExpectation:expectedValue];
    return valueExpectation.valueSatisfied;
}

- (BOOL)allValuesSatisfied
{
    for (MTRAwaitedAttributeState * valueExpectation in [self.valueExpectations allValues]) {
        if (!valueExpectation.valueSatisfied) {
            return NO;
        }
    }

    return YES;
}

- (void)_notifyWithError:(NSError * _Nullable)error
{
    MTRStatusCompletion completion;
    dispatch_queue_t queue;
    {
        // Ensure that we only call our completion once.
        std::lock_guard lock(_lock);
        if (!self.completion) {
            return;
        }

        completion = self.completion;
        queue = self.queue;
        self.completion = nil;
        self.queue = nil;

        if (self.expirationTimer != nil) {
            dispatch_source_cancel(self.expirationTimer);
            self.expirationTimer = nil;
        }
    }

    if (!error) {
        MTR_LOG("%@ %p wait for attribute values completed", self, self);
    } else if (error.domain == MTRErrorDomain && error.code == MTRErrorCodeTimeout) {
        MTR_LOG("%@ %p wait for attribute values timed out", self, self);
    } else if (error.domain == MTRErrorDomain && error.code == MTRErrorCodeCancelled) {
        MTR_LOG("%@ %p wait for attribute values canceled", self, self);
    } else {
        MTR_LOG("%@ %p wait for attribute values unknown error: %@", self, self, error);
    }

    dispatch_async(queue, ^{
        completion(error);
    });
}

- (void)_startTimerWithTimeout:(NSTimeInterval)timeout
{
    // Have the timer dispatch on the device queue, so we are not trying to do
    // it on the API consumer queue (which might be blocked by the API
    // consumer).
    dispatch_source_t timerSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, self.device.queue);

    // Set a timer to go off after timeout, and not repeat.
    dispatch_source_set_timer(timerSource, dispatch_time(DISPATCH_TIME_NOW, static_cast<uint64_t>(timeout * static_cast<double>(NSEC_PER_SEC))), DISPATCH_TIME_FOREVER,
        // Allow .5 seconds of leeway; should be plenty, in practice.
        static_cast<uint64_t>(0.5 * static_cast<double>(NSEC_PER_SEC)));

    mtr_weakify(self);
    dispatch_source_set_event_handler(timerSource, ^{
        dispatch_source_cancel(timerSource);
        mtr_strongify(self);
        if (self != nil) {
            [self.device _forgetAttributeWaiter:self];
            [self _notifyWithError:[MTRError errorForCHIPErrorCode:CHIP_ERROR_TIMEOUT]];
        }
    });

    {
        std::lock_guard lock(_lock);
        self.expirationTimer = timerSource;
    }

    dispatch_resume(timerSource);
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: %@>", NSStringFromClass(self.class), self.UUID];
}

@end
