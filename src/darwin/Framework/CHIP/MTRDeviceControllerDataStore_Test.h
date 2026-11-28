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

#import "MTRDeviceClusterData.h"

@class MTRCASESessionResumptionInfo;

NS_ASSUME_NONNULL_BEGIN

// The part of MTRDeviceControllerDataStore that tests use.  MTRDeviceControllerDataStore.h includes C++ headers, which tests
// cannot import, so tests see only this protocol.  MTRDeviceControllerDataStore.mm adopts it in a class extension, so the
// compiler checks that each method is implemented with a matching signature.
@protocol MTRDeviceControllerDataStoreAttributeStoreMethods <NSObject>
- (nullable MTRCASESessionResumptionInfo *)findResumptionInfoByNodeID:(NSNumber *)nodeID;
- (nullable NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *)getStoredClusterDataForNodeID:(NSNumber *)nodeID;
- (void)storeClusterData:(NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *)clusterData forNodeID:(NSNumber *)nodeID;
- (void)clearStoredClusterDataForNodeID:(NSNumber *)nodeID;
- (void)clearAllStoredClusterData;
- (nullable NSDictionary<NSString *, id> *)getStoredDeviceDataForNodeID:(NSNumber *)nodeID;
- (void)clearDeviceDataForNodeID:(NSNumber *)nodeID;

@property (readonly, nonatomic) NSArray<NSNumber *> * nodesWithStoredData;

#ifdef DEBUG
- (void)unitTestPruneEmptyStoredClusterDataBranches;
- (void)unitTestRereadNodeIndex;
#endif
- (NSString *)_endpointIndexKeyForNodeID:(NSNumber *)nodeID;
- (NSString *)_clusterIndexKeyForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID;
- (NSString *)_clusterDataKeyForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID;
- (nullable NSArray<NSNumber *> *)_fetchEndpointIndexForNodeID:(NSNumber *)nodeID;
- (nullable NSArray<NSNumber *> *)_fetchClusterIndexForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID;
- (nullable MTRDeviceClusterData *)_fetchClusterDataForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID;
@end

NS_ASSUME_NONNULL_END
