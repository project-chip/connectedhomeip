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

// For MTRDeviceDataValueDictionary:
#import "MTRDevice_Internal.h"

NS_ASSUME_NONNULL_BEGIN

#pragma mark - Declarations for internal methods

@class MTRDeviceClusterData;
// MTRDeviceControllerDataStore.h includes C++ header, and so we need to declare the methods separately
@protocol MTRDeviceControllerDataStoreAttributeStoreMethods
- (nullable NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *)getStoredClusterDataForNodeID:(NSNumber *)nodeID;
- (void)storeClusterData:(NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *)clusterData forNodeID:(NSNumber *)nodeID;
- (void)clearStoredClusterDataForNodeID:(NSNumber *)nodeID;
- (void)clearAllStoredClusterData;
- (void)unitTestPruneEmptyStoredClusterDataBranches;
- (NSString *)_endpointIndexKeyForNodeID:(NSNumber *)nodeID;
- (NSString *)_clusterIndexKeyForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID;
- (NSString *)_clusterDataKeyForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID;
- (nullable NSArray<NSNumber *> *)_fetchEndpointIndexForNodeID:(NSNumber *)nodeID;
- (nullable NSArray<NSNumber *> *)_fetchClusterIndexForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID;
- (nullable MTRDeviceClusterData *)_fetchClusterDataForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID;
@end

// Declare internal methods for testing
@interface MTRDeviceController (Test)
+ (void)forceLocalhostAdvertisingOnly;
- (void)removeDevice:(MTRDevice *)device;
@property (nonatomic, readonly, nullable) id<MTRDeviceControllerDataStoreAttributeStoreMethods> controllerDataStore;
@end

@interface MTRDevice (Test)
- (BOOL)_attributeDataValue:(NSDictionary *)one isEqualToDataValue:(NSDictionary *)theOther;
- (NSMutableArray<NSNumber *> *)arrayOfNumbersFromAttributeValue:(MTRDeviceDataValueDictionary)dataDictionary;
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
@end
#endif

NS_ASSUME_NONNULL_END
