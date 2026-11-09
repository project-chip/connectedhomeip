/**
 *    Copyright (c) 2026 Project CHIP Authors
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

// Re-exported for tests: MTRInternalDeviceState, -_attributeReadNeedsReadThroughForPath:params:, -_deviceMayBeReachable and
// -asyncWorkQueue are declared there, so the compiler checks them against MTRDevice.mm and MTRDevice_Concrete.mm.
#import "MTRDeviceClusterData.h"
#import "MTRDevice_Internal.h"

NS_ASSUME_NONNULL_BEGIN

// Test-only members of MTRDevice that MTRDevice.mm implements.  MTRDevice.mm adopts the protocol in a class extension, so the
// compiler checks that each one is implemented with a matching signature.
@protocol MTRDeviceTestMethods <NSObject>
#ifdef DEBUG
- (NSUInteger)unitTestNonnullDelegateCount;
- (NSUInteger)unitTestInterestedPathsChangedCount;
#endif
@end

// Test-only members that tests call through an MTRDevice, but that only MTRDevice_Concrete.mm implements.
// MTRDevice_Concrete.mm adopts the protocol in a class extension, so the compiler checks them the same way.
@protocol MTRDeviceConcreteTestMethods <NSObject>
- (NSMutableArray<NSNumber *> *)arrayOfNumbersFromAttributeValue:(MTRDeviceDataValueDictionary)dataDictionary;
- (void)setStorageBehaviorConfiguration:(MTRDeviceStorageBehaviorConfiguration *)storageBehaviorConfiguration;
// Must be called on the Matter queue.
- (void)_handleResubscriptionNeededWithDelay:(NSNumber *)resubscriptionDelayMs;
// Must be called on the Matter queue.
- (void)nodeMayBeAdvertisingOperational;
- (void)_handleReportBegin;
- (void)_handleAttributeReport:(NSArray<NSDictionary<NSString *, id> *> *)attributeReport fromSubscription:(BOOL)isFromSubscription;
- (void)_handleReportEnd;
- (void)setExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)values
    expectedValueInterval:(NSNumber *)expectedValueInterval
          expectedValueID:(uint64_t *)expectedValueID;
- (void)removeExpectedValuesForAttributePaths:(NSArray<MTRAttributePath *> *)attributePaths expectedValueID:(uint64_t)expectedValueID;
- (void)_handleSubscriptionReset:(NSNumber * _Nullable)retryDelay;
// MTRDevice_Concrete.mm declares reattemptingSubscription readwrite in its own class extension; declaring it
// readonly here, with the setter as a method, lets that extension keep synthesizing it.
- (void)setReattemptingSubscription:(BOOL)reattemptingSubscription;

@property (nonatomic, readonly) BOOL reattemptingSubscription;
@property (nonatomic, readonly, nullable) NSNumber * highestObservedEventNumber;

#ifdef DEBUG
- (void)_handleResubscriptionNeededWithDelayOnDeviceQueue:(NSNumber *)resubscriptionDelayMs;
- (void)unitTestInjectEventReport:(NSArray<NSDictionary<NSString *, id> *> *)eventReport;
- (void)unitTestInjectAttributeReport:(NSArray<NSDictionary<NSString *, id> *> *)attributeReport fromSubscription:(BOOL)isFromSubscription;
- (void)unitTestNoteTimeSynchronizationRepairScheduledAt:(NSDate *)date;
- (BOOL)unitTestShouldDetectTimeSynchronizationLoss;
- (NSUInteger)unitTestAttributesReportedSinceLastCheck;
- (NSUInteger)unitTestEventsReportedSinceLastCheck;
- (void)unitTestClearClusterData;
- (MTRInternalDeviceState)_getInternalState;
- (void)unitTestSetMostRecentReportTimes:(NSMutableArray<NSDate *> *)mostRecentReportTimes;
- (void)unitTestResetSubscription;
// Must be called on the Matter queue, with a subscription set up.
- (void)unitTestEndSubscriptionAsReadClientGivingUp;
- (MTRDeviceClusterData *)unitTestGetClusterDataForPath:(MTRClusterPath *)path;
- (NSSet<MTRClusterPath *> *)unitTestGetPersistedClusters;
- (BOOL)unitTestClusterHasBeenPersisted:(MTRClusterPath *)path;
- (NSUInteger)unitTestAttributeCount;
- (void)unitTestSyncRunOnDeviceQueue:(dispatch_block_t)block;
#endif
@end

@interface MTRDevice (Test) <MTRDeviceTestMethods, MTRDeviceConcreteTestMethods>
@end

NS_ASSUME_NONNULL_END
