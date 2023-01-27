/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
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

#import "MTRBaseDevice.h"
#import "MTRCluster_Internal.h"
#import "NSDataSpanConversion.h"
#import "NSStringSpanConversion.h"

using namespace ::chip;

@implementation MTRCluster
- (instancetype)initWithQueue:(dispatch_queue_t)queue
{
    if (self = [super init]) {
        _callbackQueue = queue;
    }
    return self;
}

- (chip::ByteSpan)asByteSpan:(NSData *)value
{
    return AsByteSpan(value);
}

- (chip::CharSpan)asCharSpan:(NSString *)value
{
    return AsCharSpan(value);
}
@end

@implementation MTRWriteParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedWriteTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRWriteParams alloc] init];
    other.timedWriteTimeout = self.timedWriteTimeout;
    other.dataVersion = self.dataVersion;
    return other;
}

@end

@implementation MTRReadParams
- (instancetype)init
{
    if (self = [super init]) {
        _filterByFabric = YES;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRReadParams alloc] init];
    other.filterByFabric = self.filterByFabric;
    other.minEventNumber = self.minEventNumber;
    return other;
}

- (void)toReadPrepareParams:(chip::app::ReadPrepareParams &)readPrepareParams
{
    readPrepareParams.mIsFabricFiltered = self.filterByFabric;
    if (self.minEventNumber) {
        readPrepareParams.mEventNumber.SetValue(static_cast<chip::EventNumber>([self.minEventNumber unsignedLongLongValue]));
    }
}

@end

@implementation MTRSubscribeParams
- (instancetype)initWithMinInterval:(NSNumber *)minInterval maxInterval:(NSNumber *)maxInterval
{
    if (self = [super init]) {
        _reportEventsUrgently = YES;
        _replaceExistingSubscriptions = YES;
        _resubscribeAutomatically = YES;
        _minInterval = [minInterval copy];
        _maxInterval = [maxInterval copy];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRSubscribeParams alloc] initWithMinInterval:self.minInterval maxInterval:self.maxInterval];
    other.filterByFabric = self.filterByFabric;
    other.minEventNumber = self.minEventNumber;
    other.replaceExistingSubscriptions = self.replaceExistingSubscriptions;
    other.reportEventsUrgently = self.reportEventsUrgently;
    other.resubscribeAutomatically = self.resubscribeAutomatically;
    return other;
}

- (void)toReadPrepareParams:(chip::app::ReadPrepareParams &)readPrepareParams
{
    [super toReadPrepareParams:readPrepareParams];
    readPrepareParams.mMinIntervalFloorSeconds = self.minInterval.unsignedShortValue;
    readPrepareParams.mMaxIntervalCeilingSeconds = self.maxInterval.unsignedShortValue;
    readPrepareParams.mKeepSubscriptions = !self.replaceExistingSubscriptions;
}

@end

@implementation MTRReadParams (Deprecated)

- (void)setFabricFiltered:(nullable NSNumber *)fabricFiltered
{
    if (fabricFiltered == nil) {
        self.filterByFabric = YES;
    } else {
        self.filterByFabric = [fabricFiltered boolValue];
    }
}

- (nullable NSNumber *)fabricFiltered
{
    return @(self.filterByFabric);
}

@end

@implementation MTRSubscribeParams (Deprecated)

- (instancetype)init
{
    if (self = [super init]) {
        _replaceExistingSubscriptions = YES;
        _resubscribeAutomatically = YES;
        _minInterval = @(1);
        _maxInterval = @(0);
    }
    return self;
}

+ (instancetype)new
{
    return [[self alloc] init];
}

- (void)setKeepPreviousSubscriptions:(nullable NSNumber *)keepPreviousSubscriptions
{
    if (keepPreviousSubscriptions == nil) {
        self.replaceExistingSubscriptions = YES;
    } else {
        self.replaceExistingSubscriptions = ![keepPreviousSubscriptions boolValue];
    }
}

- (nullable NSNumber *)keepPreviousSubscriptions
{
    return @(!self.replaceExistingSubscriptions);
}

- (void)setAutoResubscribe:(nullable NSNumber *)autoResubscribe
{
    if (autoResubscribe == nil) {
        self.resubscribeAutomatically = YES;
    } else {
        self.resubscribeAutomatically = [autoResubscribe boolValue];
    }
}

- (nullable NSNumber *)autoResubscribe
{
    return @(self.resubscribeAutomatically);
}

@end
