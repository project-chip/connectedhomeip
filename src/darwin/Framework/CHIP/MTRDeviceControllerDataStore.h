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
#import <Matter/MTRDefines.h>
#import <Matter/MTRDeviceController.h>
#import <Matter/MTRDevice_Internal.h>
#if MTR_PER_CONTROLLER_STORAGE_ENABLED
#import <Matter/MTRDeviceControllerStorageDelegate.h>
#else
#import "MTRDeviceControllerStorageDelegate_Wrapper.h"
#endif // MTR_PER_CONTROLLER_STORAGE_ENABLED

#include <lib/core/CHIPError.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * Interface that represents a single CASE session resumption entry.
 */
@interface MTRCASESessionResumptionInfo : NSObject <NSSecureCoding>
@property (nonatomic) NSNumber * nodeID;
@property (nonatomic) NSData * resumptionID;
@property (nonatomic) NSData * sharedSecret;
@property (nonatomic) NSSet<NSNumber *> * caseAuthenticatedTags;
@end

/**
 * Interface that wraps a type-safe API around
 * MTRDeviceControllerStorageDelegate.
 */
@interface MTRDeviceControllerDataStore : NSObject

- (nullable instancetype)initWithController:(MTRDeviceController *)controller
                            storageDelegate:(id<MTRDeviceControllerStorageDelegate>)storageDelegate
                       storageDelegateQueue:(dispatch_queue_t)storageDelegateQueue;

/**
 * Resumption info APIs.
 */
- (nullable MTRCASESessionResumptionInfo *)findResumptionInfoByNodeID:(NSNumber *)nodeID;
- (nullable MTRCASESessionResumptionInfo *)findResumptionInfoByResumptionID:(NSData *)resumptionID;
- (void)storeResumptionInfo:(MTRCASESessionResumptionInfo *)resumptionInfo;
- (void)clearAllResumptionInfo;

/**
 * Storage of the last NOC we used on this device.  This is local-only storage,
 * because it's used to invalidate (or not) the local-only session resumption
 * storage.
 */
- (CHIP_ERROR)storeLastLocallyUsedNOC:(MTRCertificateTLVBytes)noc;
- (MTRCertificateTLVBytes _Nullable)fetchLastLocallyUsedNOC;

/**
 * Storage for MTRDevice attribute read cache. This is local-only storage as an optimization. New controller devices using MTRDevice API can prime their own local cache from devices directly.
 */
- (nullable NSArray<NSDictionary *> *)getStoredAttributesForNodeID:(NSNumber *)nodeID;
- (nullable NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *)getStoredClusterDataForNodeID:(NSNumber *)nodeID;
- (void)storeAttributeValues:(NSArray<NSDictionary *> *)dataValues forNodeID:(NSNumber *)nodeID;
- (void)storeClusterData:(NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *)clusterData forNodeID:(NSNumber *)nodeID;
- (void)clearStoredAttributesForNodeID:(NSNumber *)nodeID;
- (void)clearAllStoredAttributes;

@end

NS_ASSUME_NONNULL_END
