//
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

#import "MTRDefines_Internal.h"
#import "MTRDeviceClusterData.h"
#import "MTRDevice_Internal.h"

NS_ASSUME_NONNULL_BEGIN

#pragma mark - Declarations for internal methods

@class MTRCASESessionResumptionInfo;

// MTRDeviceControllerDataStore.h includes C++ header, and so we need to declare the methods separately
@protocol MTRDeviceControllerDataStoreAttributeStoreMethods
- (nullable MTRCASESessionResumptionInfo *)findResumptionInfoByNodeID:(NSNumber *)nodeID;
- (nullable NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *)getStoredClusterDataForNodeID:(NSNumber *)nodeID;
- (void)storeClusterData:(NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *)clusterData forNodeID:(NSNumber *)nodeID;
- (void)clearStoredClusterDataForNodeID:(NSNumber *)nodeID;
- (void)clearAllStoredClusterData;
- (void)unitTestPruneEmptyStoredClusterDataBranches;
- (void)unitTestRereadNodeIndex;
- (NSString *)_endpointIndexKeyForNodeID:(NSNumber *)nodeID;
- (NSString *)_clusterIndexKeyForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID;
- (NSString *)_clusterDataKeyForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID;
- (nullable NSArray<NSNumber *> *)_fetchEndpointIndexForNodeID:(NSNumber *)nodeID;
- (nullable NSArray<NSNumber *> *)_fetchClusterIndexForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID;
- (nullable MTRDeviceClusterData *)_fetchClusterDataForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID;
- (nullable NSDictionary<NSString *, id> *)getStoredDeviceDataForNodeID:(NSNumber *)nodeID;
- (void)clearDeviceDataForNodeID:(NSNumber *)nodeID;

@property (readonly, nonatomic) NSArray<NSNumber *> * nodesWithStoredData;
@end

// Declare internal methods for testing
@interface MTRDeviceController (Test)
+ (void)forceLocalhostAdvertisingOnly;
- (void)removeDevice:(MTRDevice *)device;
- (void)syncRunOnWorkQueue:(void (^)(void))block error:(NSError * __autoreleasing *)error;
@property (nonatomic, readonly, nullable) id<MTRDeviceControllerDataStoreAttributeStoreMethods> controllerDataStore;
@property (nonatomic, readonly) MTRAsyncWorkQueue<MTRDeviceController *> * concurrentSubscriptionPool;
@end

@interface MTRDevice (Test)
- (NSMutableArray<NSNumber *> *)arrayOfNumbersFromAttributeValue:(MTRDeviceDataValueDictionary)dataDictionary;
- (void)setStorageBehaviorConfiguration:(MTRDeviceStorageBehaviorConfiguration *)storageBehaviorConfiguration;
- (void)_deviceMayBeReachable;
- (void)nodeMayBeAdvertisingOperational;
- (void)_handleResubscriptionNeededWithDelayOnDeviceQueue:(NSNumber *)resubscriptionDelayMs;

@property (nonatomic, readonly, nullable) NSNumber * highestObservedEventNumber;
@property (nonatomic, readonly) MTRAsyncWorkQueue<MTRDevice *> * asyncWorkQueue;
@end

#pragma mark - Declarations for items compiled only for DEBUG configuration

#ifdef DEBUG
@interface MTRDeviceController (TestDebug)
- (NSDictionary<NSNumber *, NSNumber *> *)unitTestGetDeviceAttributeCounts;
- (NSUInteger)unitTestDelegateCount;
@end

@interface MTRBaseDevice (TestDebug)
// Test function for whitebox testing
+ (id)CHIPEncodeAndDecodeNSObject:(id)object;
@end

@interface MTRDevice (TestDebug)
- (void)unitTestInjectEventReport:(NSArray<NSDictionary<NSString *, id> *> *)eventReport;
- (void)unitTestInjectAttributeReport:(NSArray<NSDictionary<NSString *, id> *> *)attributeReport fromSubscription:(BOOL)isFromSubscription;
- (NSUInteger)unitTestAttributesReportedSinceLastCheck;
- (NSUInteger)unitTestEventsReportedSinceLastCheck;
- (void)unitTestClearClusterData;
- (MTRInternalDeviceState)_getInternalState;
- (void)unitTestSetReportToPersistenceDelayTime:(NSTimeInterval)reportToPersistenceDelayTime
                reportToPersistenceDelayTimeMax:(NSTimeInterval)reportToPersistenceDelayTimeMax
                      recentReportTimesMaxCount:(NSUInteger)recentReportTimesMaxCount
            timeBetweenReportsTooShortThreshold:(NSTimeInterval)timeBetweenReportsTooShortThreshold
         timeBetweenReportsTooShortMinThreshold:(NSTimeInterval)timeBetweenReportsTooShortMinThreshold
          reportToPersistenceDelayMaxMultiplier:(double)reportToPersistenceDelayMaxMultiplier
    deviceReportingExcessivelyIntervalThreshold:(NSTimeInterval)deviceReportingExcessivelyIntervalThreshold;
- (void)unitTestSetMostRecentReportTimes:(NSMutableArray<NSDate *> *)mostRecentReportTimes;
- (NSUInteger)unitTestNonnullDelegateCount;
- (void)unitTestResetSubscription;
- (MTRDeviceClusterData *)unitTestGetClusterDataForPath:(MTRClusterPath *)path;
- (NSSet<MTRClusterPath *> *)unitTestGetPersistedClusters;
- (BOOL)unitTestClusterHasBeenPersisted:(MTRClusterPath *)path;
- (NSUInteger)unitTestAttributeCount;
- (void)unitTestSyncRunOnDeviceQueue:(dispatch_block_t)block;
// Test SPIs for the duplicate-operational-advertisement CASE self-cancel fix.
// Drive _handleSubscriptionReset: with a controlled starting backoff and
// optionally a synthetic (MTRErrorDomain, MTRErrorCodeCancelled) error; returns
// the resulting _lastSubscriptionAttemptWait.
- (uint32_t)unitTestInjectSubscriptionResetWithStartingBackoffSeconds:(uint32_t)startingBackoff
                                                      injectCancelled:(BOOL)injectCancelled;
// Drive the real OnError-stash -> OnDone-consume hand-off (the fragile ivar
// path) end to end; returns the resulting _lastSubscriptionAttemptWait and
// reports via stashClearedByOnDone whether OnDone cleared the stash.
- (uint32_t)unitTestInjectOnErrorThenOnDoneWithStartingBackoffSeconds:(uint32_t)startingBackoff
                                                      injectCancelled:(BOOL)injectCancelled
                                                 stashClearedByOnDone:(BOOL *)stashClearedByOnDone;
// Read the in-flight CASE guard (verifies it is armed for the advertisement path).
- (BOOL)unitTestDoingCASEAttemptForNodeLikelyReachable;
// Read the in-flight CASE guard after draining the Matter queue, so the read is
// ordered after the subscription-established block that clears it (that block
// runs on the Matter queue, not the device work queue).
- (BOOL)unitTestDoingCASEAttemptForNodeLikelyReachableSyncedOnMatterQueue;
// Read the current stored subscription backoff (_lastSubscriptionAttemptWait).
- (uint32_t)unitTestLastSubscriptionAttemptWait;
// Deterministic prong-1 pin: reset the subscription, drive
// _setupSubscriptionWithReason: with the given reason, and return whether the
// in-flight CASE guard was armed.  Pre-fix this returned NO for the operational
// advertisement reason (guard armed only for the SPI-hint reason); post-fix it
// returns YES for any CASE-launching reason.
- (BOOL)unitTestArmAndReadCASEGuardForSetupReason:(NSString *)reason;
@end
#endif

NS_ASSUME_NONNULL_END
