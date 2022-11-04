/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
        _fabricFiltered = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
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

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MTRSubscribeParams alloc] init];
    other.fabricFiltered = self.fabricFiltered;
    other.keepPreviousSubscriptions = self.keepPreviousSubscriptions;
    other.autoResubscribe = self.autoResubscribe;
    return other;
}

@end
