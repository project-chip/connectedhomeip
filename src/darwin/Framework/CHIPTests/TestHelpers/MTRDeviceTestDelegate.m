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
    // Snapshot atomic block-property to a local before nil-check + invoke,
    // so the getter's block-copy can't race a concurrent writer that resets
    // the property to nil between the check and the call.
    if (state == MTRDeviceStateReachable) {
        dispatch_block_t block = self.onReachable;
        if (block != nil) {
            block();
        }
    } else {
        dispatch_block_t block = self.onNotReachable;
        if (block != nil) {
            block();
        }
    }
}

- (void)_deviceInternalStateChanged:(MTRDevice *)device
{
    dispatch_block_t block = self.onInternalStateChanged;
    if (block != nil) {
        block();
    }
}

- (void)device:(MTRDevice *)device receivedAttributeReport:(NSArray<NSDictionary<NSString *, id> *> *)attributeReport
{
    MTRDeviceTestDelegateDataHandler handler = self.onAttributeDataReceived;
    if (handler != nil) {
        handler(attributeReport);
    }
}

- (void)device:(MTRDevice *)device receivedEventReport:(NSArray<NSDictionary<NSString *, id> *> *)eventReport
{
    MTRDeviceTestDelegateDataHandler handler = self.onEventDataReceived;
    if (handler != nil) {
        handler(eventReport);
    }
}

- (void)unitTestReportBeginForDevice:(MTRDevice *)device
{
    dispatch_block_t block = self.onReportBegin;
    if (block != nil) {
        block();
    }
}

- (void)unitTestReportEndForDevice:(MTRDevice *)device
{
    dispatch_block_t block = self.onReportEnd;
    if (block != nil) {
        block();
    }
}

- (NSNumber *)unitTestMaxIntervalOverrideForSubscription:(MTRDevice *)device
{
    if (self.subscriptionMaxIntervalOverride) {
        return self.subscriptionMaxIntervalOverride;
    }

    // Make sure our subscriptions time out in finite time.
    return @(2); // seconds
}

- (void)deviceCachePrimed:(MTRDevice *)device
{
    dispatch_block_t block = self.onDeviceCachePrimed;
    if (block != nil) {
        block();
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
    dispatch_block_t block = self.onDeviceConfigurationChanged;
    if (block != nil) {
        block();
    }
}

- (BOOL)unitTestPretendThreadEnabled:(MTRDevice *)device
{
    return self.pretendThreadEnabled;
}

// Tests that mock Thread (pretendThreadEnabled) but want to opt out of the
// 1s first-Thread-subscribe coldstart deferral can set
// suppressFirstThreadSubscribeDeferral = YES.  Subscription-pool ordering
// tests in MTRPerControllerStorageTests rely on the pre-deferral scheduling
// behavior; the deferral tests in MTRDeviceTests do not set this flag and so
// continue to exercise the deferral path.  See PR #72268.
- (BOOL)unitTestSuppressFirstThreadSubscribeDeferral:(MTRDevice *)device
{
    return self.suppressFirstThreadSubscribeDeferral;
}

- (void)unitTestSubscriptionPoolDequeue:(MTRDevice *)device
{
    dispatch_block_t block = self.onSubscriptionPoolDequeue;
    if (block != nil) {
        block();
    }
}

- (void)unitTestSubscriptionPoolWorkComplete:(MTRDevice *)device
{
    dispatch_block_t block = self.onSubscriptionPoolWorkComplete;
    if (block != nil) {
        block();
    }
}

- (void)unitTestClusterDataPersisted:(MTRDevice *)device
{
    dispatch_block_t block = self.onClusterDataPersisted;
    if (block != nil) {
        block();
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
    dispatch_block_t block = self.onSubscriptionCallbackDelete;
    if (block != nil) {
        block();
    }
}

- (void)unitTestSubscriptionResetForDevice:(MTRDevice *)device
{
    dispatch_block_t block = self.onSubscriptionReset;
    if (block != nil) {
        block();
    }
}

- (void)unitTestSetUTCTimeInvokedForDevice:(MTRDevice *)device error:(NSError * _Nullable)error
{
    MTRDeviceTestDelegateHandler handler = self.onUTCTimeSet;
    if (handler != nil) {
        handler(error);
    }
}

- (BOOL)unitTestTimeUpdateShortDelayIsZero:(MTRDevice *)device
{
    return self.forceTimeUpdateShortDelayToZero;
}

- (BOOL)unitTestTimeSynchronizationLossDetectionCadenceIsZero:(MTRDevice *)device
{
    return self.forceTimeSynchronizationLossDetectionCadenceToZero;
}

- (void)unitTestTimeSynchronizationLossDetectedForDevice:(MTRDevice *)device
{
    dispatch_block_t block = self.onTimeSynchronizationLossDetected;
    if (block != nil) {
        block();
    }
}

@end

@implementation MTRDeviceTestDelegateWithSubscriptionSetupOverride

- (BOOL)unitTestShouldSetUpSubscriptionForDevice:(MTRDevice *)device
{
    return !_skipSetupSubscription;
}

@end
