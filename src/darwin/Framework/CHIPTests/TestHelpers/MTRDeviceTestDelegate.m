/**
 *    Copyright (c) 2023 Project CHIP Authors
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

#import "MTRDeviceTestDelegate.h"

@implementation MTRDeviceTestDelegate
- (void)device:(MTRDevice *)device stateChanged:(MTRDeviceState)state
{
    if (state == MTRDeviceStateReachable && self.onReachable != nil) {
        self.onReachable();
    } else if (state != MTRDeviceStateReachable && self.onNotReachable != nil) {
        self.onNotReachable();
    }
}

- (void)device:(MTRDevice *)device receivedAttributeReport:(NSArray<NSDictionary<NSString *, id> *> *)attributeReport
{
    if (self.onAttributeDataReceived != nil) {
        self.onAttributeDataReceived(attributeReport);
    }
}

- (void)device:(MTRDevice *)device receivedEventReport:(NSArray<NSDictionary<NSString *, id> *> *)eventReport
{
    if (self.onEventDataReceived != nil) {
        self.onEventDataReceived(eventReport);
    }
}

- (void)unitTestReportEndForDevice:(MTRDevice *)device
{
    if (self.onReportEnd != nil) {
        self.onReportEnd();
    }
}

- (NSNumber *)unitTestMaxIntervalOverrideForSubscription:(MTRDevice *)device
{
    // Make sure our subscriptions time out in finite time.
    return @(2); // seconds
}

- (void)deviceCachePrimed:(MTRDevice *)device
{
    if (self.onDeviceCachePrimed != nil) {
        self.onDeviceCachePrimed();
    }
}

@end
