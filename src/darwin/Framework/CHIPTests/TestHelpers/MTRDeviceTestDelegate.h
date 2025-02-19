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

@interface MTRDeviceTestDelegate : NSObject <MTRDeviceDelegate>
@property (nonatomic, nullable) dispatch_block_t onReachable;
@property (nonatomic, nullable) dispatch_block_t onNotReachable;
@property (nonatomic, nullable) MTRDeviceTestDelegateDataHandler onAttributeDataReceived;
@property (nonatomic, nullable) MTRDeviceTestDelegateDataHandler onEventDataReceived;
@property (nonatomic, nullable) dispatch_block_t onReportBegin;
@property (nonatomic, nullable) dispatch_block_t onReportEnd;
@property (nonatomic, nullable) dispatch_block_t onDeviceCachePrimed;
@property (nonatomic) BOOL skipExpectedValuesForWrite;
@property (nonatomic) BOOL forceAttributeReportsIfMatchingCache;
@property (nonatomic, nullable) dispatch_block_t onDeviceConfigurationChanged;
@property (nonatomic) BOOL pretendThreadEnabled;
@property (nonatomic, nullable) dispatch_block_t onSubscriptionPoolDequeue;
@property (nonatomic, nullable) dispatch_block_t onSubscriptionPoolWorkComplete;
@property (nonatomic, nullable) dispatch_block_t onClusterDataPersisted;
@property (nonatomic, nullable) dispatch_block_t onSubscriptionCallbackDelete;
@property (nonatomic, nullable) dispatch_block_t onSubscriptionReset;
@end

@interface MTRDeviceTestDelegateWithSubscriptionSetupOverride : MTRDeviceTestDelegate
@property (nonatomic) BOOL skipSetupSubscription;
@end

NS_ASSUME_NONNULL_END
