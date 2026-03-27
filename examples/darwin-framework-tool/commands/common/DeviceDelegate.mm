/*
 *   Copyright (c) 2024 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#import "DeviceDelegate.h"

#include <lib/support/logging/CHIPLogging.h>

NS_ASSUME_NONNULL_BEGIN

@interface DeviceDelegate ()
@property (nonatomic, readwrite) NSNumber * maxIntervalForSubscription;
@property (nonatomic, readwrite) BOOL threadEnabled;
@end

@implementation DeviceDelegate
- (instancetype)init
{
    if (self = [super init]) {
        _maxIntervalForSubscription = nil;
        _threadEnabled = NO;
        _onDeviceCachePrimed = nil;
    }
    return self;
}

- (void)device:(MTRDevice *)device stateChanged:(MTRDeviceState)state
{
}

- (void)device:(MTRDevice *)device receivedAttributeReport:(NSArray<NSDictionary<NSString *, id> *> *)attributeReport
{
}

- (void)device:(MTRDevice *)device receivedEventReport:(NSArray<NSDictionary<NSString *, id> *> *)eventReport
{
}

- (void)deviceCachePrimed:(MTRDevice *)device
{
    if (self.onDeviceCachePrimed != nil) {
        self.onDeviceCachePrimed();
    }
}

- (void)deviceConfigurationChanged:(MTRDevice *)device
{
}

- (void)setMaxIntervalForSubscription:(NSNumber *)maxInterval
{
    _maxIntervalForSubscription = maxInterval;
}

- (NSNumber *)unitTestMaxIntervalOverrideForSubscription:(MTRDevice *)device
{
    return _maxIntervalForSubscription;
}

- (void)setPretendThreadEnabled:(BOOL)threadEnabled
{
    _threadEnabled = threadEnabled;
}

- (BOOL)unitTestPretendThreadEnabled:(MTRDevice *)device
{
    return _threadEnabled;
}
@end

NS_ASSUME_NONNULL_END
