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

- (void)unitTestReportBeginForDevice:(MTRDevice *)device
{
    if (self.onReportBegin != nil) {
        self.onReportBegin();
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

- (BOOL)unitTestShouldSkipExpectedValuesForWrite:(MTRDevice *)device
{
    return self.skipExpectedValuesForWrite;
}

- (BOOL)unitTestForceAttributeReportsIfMatchingCache:(MTRDevice *)device
{
    return self.forceAttributeReportsIfMatchingCache;
}

- (void)deviceConfigurationChanged:(MTRDevice *)device
{
    if (self.onDeviceConfigurationChanged != nil) {
        self.onDeviceConfigurationChanged();
    }
}

- (BOOL)unitTestPretendThreadEnabled:(MTRDevice *)device
{
    return self.pretendThreadEnabled;
}

- (void)unitTestSubscriptionPoolDequeue:(MTRDevice *)device
{
    if (self.onSubscriptionPoolDequeue != nil) {
        self.onSubscriptionPoolDequeue();
    }
}

- (void)unitTestSubscriptionPoolWorkComplete:(MTRDevice *)device
{
    if (self.onSubscriptionPoolWorkComplete != nil) {
        self.onSubscriptionPoolWorkComplete();
    }
}

- (void)unitTestClusterDataPersisted:(MTRDevice *)device
{
    if (self.onClusterDataPersisted != nil) {
        self.onClusterDataPersisted();
    }
}

- (BOOL)unitTestSuppressTimeBasedReachabilityChanges:(MTRDevice *)device
{
    // Allowing time-based reachability changes just makes the tests
    // non-deterministic and can lead to random failures.  Suppress them
    // unconditionally for now.  If we ever add tests that try to exercise that
    // codepath, we can make this configurable.
    return YES;
}

- (void)unitTestSubscriptionCallbackDeleteForDevice:(MTRDevice *)device
{
    if (self.onSubscriptionCallbackDelete != nil) {
        self.onSubscriptionCallbackDelete();
    }
}

- (void)unitTestSubscriptionResetForDevice:(MTRDevice *)device
{
    if (self.onSubscriptionReset != nil) {
        self.onSubscriptionReset();
    }
}

@end

@implementation MTRDeviceTestDelegateWithSubscriptionSetupOverride

- (BOOL)unitTestShouldSetUpSubscriptionForDevice:(MTRDevice *)device
{
    return !_skipSetupSubscription;
}

@end
