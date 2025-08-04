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
@property (atomic, nullable) NSNumber * subscriptionMaxIntervalOverride;
@property (atomic, copy, nullable) MTRDeviceTestDelegateHandler onUTCTimeSet;
@property (atomic) BOOL forceTimeUpdateShortDelayToZero;
@property (atomic) BOOL forceTimeSynchronizationLossDetectionCadenceToZero;
@end

@interface MTRDeviceTestDelegateWithSubscriptionSetupOverride : MTRDeviceTestDelegate
@property (atomic) BOOL skipSetupSubscription;
@end

NS_ASSUME_NONNULL_END
