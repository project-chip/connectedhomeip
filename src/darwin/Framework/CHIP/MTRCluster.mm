/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRBaseDevice.h"
#import "MTRCluster_internal.h"
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

- (id)copyWithZone:(nullable NSZone *)zone
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
        _fabricFiltered = nil;
    }
    return self;
}

- (id)copyWithZone:(nullable NSZone *)zone
{
    auto other = [[MTRReadParams alloc] init];
    other.fabricFiltered = self.fabricFiltered;
    return other;
}

@end

@implementation MTRSubscribeParams
- (instancetype)init
{
    if (self = [super init]) {
        _keepPreviousSubscriptions = nil;
        _autoResubscribe = nil;
    }
    return self;
}

- (id)copyWithZone:(nullable NSZone *)zone
{
    auto other = [[MTRSubscribeParams alloc] init];
    other.fabricFiltered = self.fabricFiltered;
    other.keepPreviousSubscriptions = self.keepPreviousSubscriptions;
    other.autoResubscribe = self.autoResubscribe;
    return other;
}

@end
