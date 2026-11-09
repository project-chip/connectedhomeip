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

#import <Foundation/Foundation.h>
#import <Matter/Matter.h>

NS_ASSUME_NONNULL_BEGIN

typedef void (^MTRDeviceTestDelegateDataHandler)(NSArray<NSDictionary<NSString *, id> *> *);
typedef void (^MTRDeviceTestDelegateHandler)(NSError * error);

@interface MTRDeviceTestDelegate : NSObject <MTRDeviceDelegate>
@property (atomic, copy, nullable) dispatch_block_t onReachable;
@property (atomic, copy, nullable) dispatch_block_t onNotReachable;
@property (atomic, copy, nullable) dispatch_block_t onInternalStateChanged;
@property (atomic, copy, nullable) MTRDeviceTestDelegateDataHandler onAttributeDataReceived;
@property (atomic, copy, nullable) MTRDeviceTestDelegateDataHandler onEventDataReceived;
@property (atomic, copy, nullable) dispatch_block_t onReportBegin;
@property (atomic, copy, nullable) dispatch_block_t onReportEnd;
@property (atomic, copy, nullable) dispatch_block_t onDeviceCachePrimed;
@property (atomic) BOOL skipExpectedValuesForWrite;
@property (atomic) BOOL forceAttributeReportsIfMatchingCache;
@property (atomic, copy, nullable) dispatch_block_t onDeviceConfigurationChanged;
@property (atomic) BOOL pretendThreadEnabled;
@property (atomic, copy, nullable) dispatch_block_t onSubscriptionPoolDequeue;
@property (atomic, copy, nullable) dispatch_block_t onSubscriptionPoolWorkComplete;
@property (atomic, copy, nullable) dispatch_block_t onClusterDataPersisted;
@property (atomic, copy, nullable) dispatch_block_t onSubscriptionCallbackDelete;
@property (atomic, copy, nullable) dispatch_block_t onSubscriptionReset;
// Called on the Matter queue, with the device lock held, when the device starts a new subscription.
@property (atomic, copy, nullable) dispatch_block_t onSubscriptionSetupStarted;
@property (atomic, copy, nullable) dispatch_block_t onReadClientCreated;
// Called synchronously on the Matter queue, with the device lock held, for each internal state change.
@property (atomic, copy, nullable) void (^onInternalStateTransition)(NSUInteger state);
@property (atomic, nullable) NSNumber * subscriptionMaxIntervalOverride;
@property (atomic, nullable) NSNumber * resubscriptionDelayOverrideMs;
// Replaces the backoff MTRDevice waits before reattempting a subscription that failed to set up.
@property (atomic, nullable) NSNumber * subscriptionResetDelayOverrideMs;
// Called once the subscription's session request has been queued on the Matter queue.
@property (atomic, copy, nullable) dispatch_block_t onSubscriptionSessionRequestStarted;
// When set, called when the subscription's session request fails; its result replaces the request's retry delay, in
// seconds, as a server-provided busy delay would.
@property (atomic, copy, nullable) NSNumber * _Nullable (^onSessionRequestFailedRetryDelay)(void);
@property (atomic, copy, nullable) dispatch_block_t onResubscriptionDelayRequested;
// When YES, the device ignores operational advertisements, so only other signals can trigger a resubscribe.
@property (atomic) BOOL ignoreOperationalAdvertisements;
@property (atomic, copy, nullable) MTRDeviceTestDelegateHandler onUTCTimeSet;
@property (atomic, copy, nullable) dispatch_block_t onTimeSynchronizationLossDetected;
@property (atomic) BOOL forceTimeUpdateShortDelayToZero;
@property (atomic, nullable) NSNumber * timeSynchronizationLossDetectionCadenceOverride;
@end

@interface MTRDeviceTestDelegateWithSubscriptionSetupOverride : MTRDeviceTestDelegate
@property (atomic) BOOL skipSetupSubscription;
@end

NS_ASSUME_NONNULL_END
