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

NS_ASSUME_NONNULL_BEGIN

#pragma mark - Declarations for internal methods

@class MTRDeviceClusterData;
// MTRDeviceControllerDataStore.h includes C++ header, and so we need to declare the methods separately
@protocol MTRDeviceControllerDataStoreAttributeStoreMethods
- (nullable NSArray<NSDictionary *> *)getStoredAttributesForNodeID:(NSNumber *)nodeID;
- (nullable NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *)getStoredClusterDataForNodeID:(NSNumber *)nodeID;
- (void)storeAttributeValues:(NSArray<NSDictionary *> *)dataValues forNodeID:(NSNumber *)nodeID;
- (void)storeClusterData:(NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *)clusterData forNodeID:(NSNumber *)nodeID;
- (void)clearStoredAttributesForNodeID:(NSNumber *)nodeID;
- (void)clearAllStoredAttributes;
- (void)unitTestPruneEmptyStoredAttributesBranches;
- (NSString *)_endpointIndexKeyForNodeID:(NSNumber *)nodeID;
- (NSString *)_clusterIndexKeyForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID;
- (NSString *)_attributeIndexKeyForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID;
- (NSString *)_attributeValueKeyForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID attributeID:(NSNumber *)attributeID;
@end

// Declare internal methods for testing
@interface MTRDeviceController (Test)
+ (void)forceLocalhostAdvertisingOnly;
- (void)removeDevice:(MTRDevice *)device;
@property (nonatomic, readonly, nullable) id<MTRDeviceControllerDataStoreAttributeStoreMethods> controllerDataStore;
@end

@interface MTRDevice (Test)
- (BOOL)_attributeDataValue:(NSDictionary *)one isEqualToDataValue:(NSDictionary *)theOther;
@end

#pragma mark - Declarations for items compiled only for DEBUG configuration

#ifdef DEBUG
@interface MTRBaseDevice (TestDebug)
- (void)failSubscribers:(dispatch_queue_t)queue completion:(void (^)(void))completion;

// Test function for whitebox testing
+ (id)CHIPEncodeAndDecodeNSObject:(id)object;
@end

@interface MTRDevice (TestDebug)
- (void)unitTestInjectEventReport:(NSArray<NSDictionary<NSString *, id> *> *)eventReport;
- (NSUInteger)unitTestAttributesReportedSinceLastCheck;
@end
#endif

NS_ASSUME_NONNULL_END
