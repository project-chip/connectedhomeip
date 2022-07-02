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

#import "MTRCluster_internal.h"
#import "MTRDevice.h"
#import "NSDataSpanConversion.h"

using namespace ::chip;

@implementation MTRCluster
- (instancetype)initWithDevice:(MTRDevice *)device endpoint:(EndpointId)endpoint queue:(dispatch_queue_t)queue
{
    if (self = [super init]) {
        Controller::ClusterBase * cppCluster = [self getCluster];
        if (cppCluster == nullptr) {
            return nil;
        }

        if (device == nullptr) {
            return nil;
        }

        CHIP_ERROR err = cppCluster->Associate([device internalDevice], endpoint);
        if (err != CHIP_NO_ERROR) {
            return nil;
        }

        _callbackQueue = queue;
    }
    return self;
}

- (Controller::ClusterBase *)getCluster
{
    return nullptr;
}

- (chip::ByteSpan)asByteSpan:(NSData *)value
{
    return AsByteSpan(value);
}

- (chip::CharSpan)asCharSpan:(NSString *)value
{
    return chip::CharSpan(static_cast<const char *>([value dataUsingEncoding:NSUTF8StringEncoding].bytes),
        [value lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
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

@end

@implementation MTRReadParams
- (instancetype)init
{
    if (self = [super init]) {
        _fabricFiltered = nil;
    }
    return self;
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

@end
