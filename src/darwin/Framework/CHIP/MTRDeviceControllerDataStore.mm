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

#include "MTRDeviceControllerDataStore.h"

// Importing MTRBaseDevice.h for the MTRAttributePath class. Needs to change when https://github.com/project-chip/connectedhomeip/issues/31247 is fixed.
#import "MTRBaseDevice.h"
#import "MTRLogging_Internal.h"

#include <lib/core/CASEAuthTag.h>
#include <lib/core/NodeId.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>

// FIXME: Are these good key strings? https://github.com/project-chip/connectedhomeip/issues/28973
static NSString * sResumptionNodeListKey = @"caseResumptionNodeList";
static NSString * sLastLocallyUsedNOCKey = @"lastLocallyUsedControllerNOC";

static NSString * ResumptionByNodeIDKey(NSNumber * nodeID)
{
    return [NSString stringWithFormat:@"caseResumptionByNodeID/%llx", nodeID.unsignedLongLongValue];
}

static NSString * ResumptionByResumptionIDKey(NSData * resumptionID)
{
    return
        [NSString stringWithFormat:@"caseResumptionByResumptionID/%s", [resumptionID base64EncodedStringWithOptions:0].UTF8String];
}

static bool IsUnsignedIntegerNumber(id _Nullable value)
{
    if (value == nil) {
        return false;
    }

    if (![value isKindOfClass:[NSNumber class]]) {
        return false;
    }

    NSNumber * number = value;

    // Not sure how to check for the number being an integer.

    if ([number compare:@(0)] == NSOrderedAscending) {
        return false;
    }

    return true;
}

static bool IsValidNodeIDNumber(id _Nullable value)
{
    // Node IDs cannot be negative.
    if (!IsUnsignedIntegerNumber(value)) {
        return false;
    }

    NSNumber * number = value;

    // Validate that this is a valid operational ID, not some garbage unsigned
    // int value that can't be a node id.
    uint64_t unsignedValue = number.unsignedLongLongValue;
    if (!chip::IsOperationalNodeId(unsignedValue)) {
        return false;
    }

    return true;
}

static bool IsValidCATNumber(id _Nullable value)
{
    // CATs cannot be negative.
    if (!IsUnsignedIntegerNumber(value)) {
        return false;
    }

    NSNumber * number = value;

    // Validate that this is a valid CAT value and, not some garbage unsigned int
    // value that can't be a CAT.
    uint64_t unsignedValue = number.unsignedLongLongValue;
    if (!chip::CanCastTo<chip::CASEAuthTag>(unsignedValue)) {
        return false;
    }

    auto tag = static_cast<chip::CASEAuthTag>(unsignedValue);
    if (!chip::IsValidCASEAuthTag(tag)) {
        return false;
    }

    return true;
}

@implementation MTRDeviceControllerDataStore {
    id<MTRDeviceControllerStorageDelegate> _storageDelegate;
    dispatch_queue_t _storageDelegateQueue;
    // Controller owns us, so we have to make sure to not keep it alive.
    __weak MTRDeviceController * _controller;
    // Array of nodes with resumption info, oldest-stored first.
    NSMutableArray<NSNumber *> * _nodesWithResumptionInfo;
}

- (nullable instancetype)initWithController:(MTRDeviceController *)controller
                            storageDelegate:(id<MTRDeviceControllerStorageDelegate>)storageDelegate
                       storageDelegateQueue:(dispatch_queue_t)storageDelegateQueue
{
    if (!(self = [super init])) {
        return nil;
    }

    _controller = controller;
    _storageDelegate = storageDelegate;
    _storageDelegateQueue = storageDelegateQueue;

    __block id resumptionNodeList;
    dispatch_sync(_storageDelegateQueue, ^{
        @autoreleasepool {
            // NOTE: controller, not our weak ref, since we know it's still
            // valid under this sync dispatch.
            resumptionNodeList = [_storageDelegate controller:controller
                                                  valueForKey:sResumptionNodeListKey
                                                securityLevel:MTRStorageSecurityLevelSecure
                                                  sharingType:MTRStorageSharingTypeNotShared];
        }
    });
    if (resumptionNodeList != nil) {
        if (![resumptionNodeList isKindOfClass:[NSArray class]]) {
            MTR_LOG_ERROR("List of CASE resumption node IDs is not an array");
            return nil;
        }
        for (id value in resumptionNodeList) {
            if (!IsValidNodeIDNumber(value)) {
                MTR_LOG_ERROR("Resumption node ID contains invalid value: %@", value);
                return nil;
            }
        }
        _nodesWithResumptionInfo = [resumptionNodeList mutableCopy];
    } else {
        _nodesWithResumptionInfo = [[NSMutableArray alloc] init];
    }

    return self;
}

- (void)fetchAttributeDataForAllDevices:(MTRDeviceControllerDataStoreClusterDataHandler)clusterDataHandler
{
    __block NSDictionary<NSString *, id> * dataStoreSecureLocalValues = nil;
    MTRDeviceController * controller = _controller;
    VerifyOrReturn(controller != nil); // No way to call delegate without controller.

    dispatch_sync(_storageDelegateQueue, ^{
        if ([self->_storageDelegate respondsToSelector:@selector(valuesForController:securityLevel:sharingType:)]) {
            dataStoreSecureLocalValues = [self->_storageDelegate valuesForController:controller securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared];
        }
    });

    if (dataStoreSecureLocalValues.count) {
        clusterDataHandler([self _getClusterDataFromSecureLocalValues:dataStoreSecureLocalValues]);
    }
}

- (nullable MTRCASESessionResumptionInfo *)findResumptionInfoByNodeID:(NSNumber *)nodeID
{
    return [self _findResumptionInfoWithKey:ResumptionByNodeIDKey(nodeID)];
}

- (nullable MTRCASESessionResumptionInfo *)findResumptionInfoByResumptionID:(NSData *)resumptionID
{
    return [self _findResumptionInfoWithKey:ResumptionByResumptionIDKey(resumptionID)];
}

- (void)storeResumptionInfo:(MTRCASESessionResumptionInfo *)resumptionInfo
{
    MTRDeviceController * controller = _controller;
    VerifyOrReturn(controller != nil); // No way to call delegate without controller.

    auto * oldInfo = [self findResumptionInfoByNodeID:resumptionInfo.nodeID];
    dispatch_sync(_storageDelegateQueue, ^{
        if (oldInfo != nil) {
            // Remove old resumption id key.  No need to do that for the
            // node id, because we are about to overwrite it.
            [_storageDelegate controller:controller
                       removeValueForKey:ResumptionByResumptionIDKey(oldInfo.resumptionID)
                           securityLevel:MTRStorageSecurityLevelSecure
                             sharingType:MTRStorageSharingTypeNotShared];
            [_nodesWithResumptionInfo removeObject:resumptionInfo.nodeID];
        }

        [_storageDelegate controller:controller
                          storeValue:resumptionInfo
                              forKey:ResumptionByNodeIDKey(resumptionInfo.nodeID)
                       securityLevel:MTRStorageSecurityLevelSecure
                         sharingType:MTRStorageSharingTypeNotShared];
        [_storageDelegate controller:controller
                          storeValue:resumptionInfo
                              forKey:ResumptionByResumptionIDKey(resumptionInfo.resumptionID)
                       securityLevel:MTRStorageSecurityLevelSecure
                         sharingType:MTRStorageSharingTypeNotShared];

        // Update our resumption info node list.
        [_nodesWithResumptionInfo addObject:resumptionInfo.nodeID];
        [_storageDelegate controller:controller
                          storeValue:[_nodesWithResumptionInfo copy]
                              forKey:sResumptionNodeListKey
                       securityLevel:MTRStorageSecurityLevelSecure
                         sharingType:MTRStorageSharingTypeNotShared];
    });
}

- (void)clearAllResumptionInfo
{
    MTRDeviceController * controller = _controller;
    VerifyOrReturn(controller != nil); // No way to call delegate without controller.

    // Can we do less dispatch?  We would need to have a version of
    // _findResumptionInfoWithKey that assumes we are already on the right queue.
    for (NSNumber * nodeID in _nodesWithResumptionInfo) {
        auto * oldInfo = [self findResumptionInfoByNodeID:nodeID];
        if (oldInfo != nil) {
            dispatch_sync(_storageDelegateQueue, ^{
                [_storageDelegate controller:controller
                           removeValueForKey:ResumptionByResumptionIDKey(oldInfo.resumptionID)
                               securityLevel:MTRStorageSecurityLevelSecure
                                 sharingType:MTRStorageSharingTypeNotShared];
                [_storageDelegate controller:controller
                           removeValueForKey:ResumptionByNodeIDKey(oldInfo.nodeID)
                               securityLevel:MTRStorageSecurityLevelSecure
                                 sharingType:MTRStorageSharingTypeNotShared];
            });
        }
    }

    [_nodesWithResumptionInfo removeAllObjects];
}

- (CHIP_ERROR)storeLastLocallyUsedNOC:(MTRCertificateTLVBytes)noc
{
    MTRDeviceController * controller = _controller;
    VerifyOrReturnError(controller != nil, CHIP_ERROR_PERSISTED_STORAGE_FAILED); // No way to call delegate without controller.

    __block BOOL ok;
    dispatch_sync(_storageDelegateQueue, ^{
        ok = [_storageDelegate controller:controller
                               storeValue:noc
                                   forKey:sLastLocallyUsedNOCKey
                            securityLevel:MTRStorageSecurityLevelSecure
                              sharingType:MTRStorageSharingTypeNotShared];
    });
    return ok ? CHIP_NO_ERROR : CHIP_ERROR_PERSISTED_STORAGE_FAILED;
}

- (MTRCertificateTLVBytes _Nullable)fetchLastLocallyUsedNOC
{
    MTRDeviceController * controller = _controller;
    VerifyOrReturnValue(controller != nil, nil); // No way to call delegate without controller.

    __block id data;
    dispatch_sync(_storageDelegateQueue, ^{
        @autoreleasepool {
            data = [_storageDelegate controller:controller
                                    valueForKey:sLastLocallyUsedNOCKey
                                  securityLevel:MTRStorageSecurityLevelSecure
                                    sharingType:MTRStorageSharingTypeNotShared];
        }
    });

    if (data == nil) {
        return nil;
    }

    if (![data isKindOfClass:[NSData class]]) {
        return nil;
    }

    return data;
}

- (nullable MTRCASESessionResumptionInfo *)_findResumptionInfoWithKey:(nullable NSString *)key
{
    MTRDeviceController * controller = _controller;
    VerifyOrReturnValue(controller != nil, nil); // No way to call delegate without controller.

    // key could be nil if [NSString stringWithFormat] returns nil for some reason.
    if (key == nil) {
        return nil;
    }

    __block id resumptionInfo;
    dispatch_sync(_storageDelegateQueue, ^{
        @autoreleasepool {
            resumptionInfo = [_storageDelegate controller:controller
                                              valueForKey:key
                                            securityLevel:MTRStorageSecurityLevelSecure
                                              sharingType:MTRStorageSharingTypeNotShared];
        }
    });

    if (resumptionInfo == nil) {
        return nil;
    }

    if (![resumptionInfo isKindOfClass:[MTRCASESessionResumptionInfo class]]) {
        return nil;
    }

    return resumptionInfo;
}

#pragma - Attribute Cache utility

/** MTRDevice cache storage
 *
 *  Per controller:
 *    NodeID index
 *        key: "attrCacheNodeIndex"
 *        value: list of nodeIDs
 *    EndpointID index
 *        key: "attrCacheEndpointIndex:<nodeID>"
 *        value: list of endpoint IDs
 *    ClusterID index
 *        key: "attrCacheClusterIndex:<nodeID>:<endpointID>"
 *        value: list of cluster IDs
 *    Cluster data entry:
 *        key: "attrCacheClusterData:<nodeID>:<endpointID>:<clusterID>"
 *        value: MTRDeviceClusterData
 */

- (id)_fetchAttributeCacheValueForKey:(NSString *)key expectedClass:(Class)expectedClass;
{
    MTRDeviceController * controller = _controller;
    VerifyOrReturnValue(controller != nil, nil); // No way to call delegate without controller.

    id data;
    @autoreleasepool {
        data = [_storageDelegate controller:controller
                                valueForKey:key
                              securityLevel:MTRStorageSecurityLevelSecure
                                sharingType:MTRStorageSharingTypeNotShared];
    }
    if (data == nil) {
        return nil;
    }

    if (![data isKindOfClass:expectedClass]) {
        return nil;
    }

    return data;
}

- (BOOL)_storeAttributeCacheValue:(id)value forKey:(NSString *)key
{
    MTRDeviceController * controller = _controller;
    VerifyOrReturnValue(controller != nil, NO); // No way to call delegate without controller.

    return [_storageDelegate controller:controller
                             storeValue:value
                                 forKey:key
                          securityLevel:MTRStorageSecurityLevelSecure
                            sharingType:MTRStorageSharingTypeNotShared];
}

- (BOOL)_bulkStoreAttributeCacheValues:(NSDictionary<NSString *, id<NSSecureCoding>> *)values
{
    MTRDeviceController * controller = _controller;
    VerifyOrReturnValue(controller != nil, NO); // No way to call delegate without controller.

    return [_storageDelegate controller:controller
                            storeValues:values
                          securityLevel:MTRStorageSecurityLevelSecure
                            sharingType:MTRStorageSharingTypeNotShared];
}

- (BOOL)_removeAttributeCacheValueForKey:(NSString *)key
{
    MTRDeviceController * controller = _controller;
    VerifyOrReturnValue(controller != nil, NO); // No way to call delegate without controller.

    return [_storageDelegate controller:controller
                      removeValueForKey:key
                          securityLevel:MTRStorageSecurityLevelSecure
                            sharingType:MTRStorageSharingTypeNotShared];
}

static NSString * sAttributeCacheNodeIndexKey = @"attrCacheNodeIndex";

- (nullable NSArray<NSNumber *> *)_fetchNodeIndex
{
    dispatch_assert_queue(_storageDelegateQueue);
    return [self _fetchAttributeCacheValueForKey:sAttributeCacheNodeIndexKey expectedClass:[NSArray class]];
}

- (BOOL)_storeNodeIndex:(NSArray<NSNumber *> *)nodeIndex
{
    dispatch_assert_queue(_storageDelegateQueue);
    return [self _storeAttributeCacheValue:nodeIndex forKey:sAttributeCacheNodeIndexKey];
}

- (BOOL)_deleteNodeIndex
{
    dispatch_assert_queue(_storageDelegateQueue);
    return [self _removeAttributeCacheValueForKey:sAttributeCacheNodeIndexKey];
}

static NSString * sAttributeCacheEndpointIndexKeyPrefix = @"attrCacheEndpointIndex";

- (NSString *)_endpointIndexKeyForNodeID:(NSNumber *)nodeID
{
    return [sAttributeCacheEndpointIndexKeyPrefix stringByAppendingFormat:@":0x%016llX", nodeID.unsignedLongLongValue];
}

- (nullable NSArray<NSNumber *> *)_fetchEndpointIndexForNodeID:(NSNumber *)nodeID
{
    dispatch_assert_queue(_storageDelegateQueue);

    if (!nodeID) {
        MTR_LOG_ERROR("%s: unexpected nil input", __func__);
        return nil;
    }

    return [self _fetchAttributeCacheValueForKey:[self _endpointIndexKeyForNodeID:nodeID] expectedClass:[NSArray class]];
}

- (BOOL)_storeEndpointIndex:(NSArray<NSNumber *> *)endpointIndex forNodeID:(NSNumber *)nodeID
{
    dispatch_assert_queue(_storageDelegateQueue);

    if (!nodeID) {
        MTR_LOG_ERROR("%s: unexpected nil input", __func__);
        return NO;
    }

    return [self _storeAttributeCacheValue:endpointIndex forKey:[self _endpointIndexKeyForNodeID:nodeID]];
}

- (BOOL)_deleteEndpointIndexForNodeID:(NSNumber *)nodeID
{
    dispatch_assert_queue(_storageDelegateQueue);

    if (!nodeID) {
        MTR_LOG_ERROR("%s: unexpected nil input", __func__);
        return NO;
    }

    return [self _removeAttributeCacheValueForKey:[self _endpointIndexKeyForNodeID:nodeID]];
}

static NSString * sAttributeCacheClusterIndexKeyPrefix = @"attrCacheClusterIndex";

- (NSString *)_clusterIndexKeyForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID
{
    return [sAttributeCacheClusterIndexKeyPrefix stringByAppendingFormat:@":0x%016llX:%0x04X", nodeID.unsignedLongLongValue, endpointID.unsignedShortValue];
}

- (nullable NSArray<NSNumber *> *)_fetchClusterIndexForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID
{
    dispatch_assert_queue(_storageDelegateQueue);

    if (!nodeID || !endpointID) {
        MTR_LOG_ERROR("%s: unexpected nil input", __func__);
        return nil;
    }

    return [self _fetchAttributeCacheValueForKey:[self _clusterIndexKeyForNodeID:nodeID endpointID:endpointID] expectedClass:[NSArray class]];
}

- (BOOL)_storeClusterIndex:(NSArray<NSNumber *> *)clusterIndex forNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID
{
    dispatch_assert_queue(_storageDelegateQueue);

    if (!nodeID || !endpointID) {
        MTR_LOG_ERROR("%s: unexpected nil input", __func__);
        return NO;
    }

    return [self _storeAttributeCacheValue:clusterIndex forKey:[self _clusterIndexKeyForNodeID:nodeID endpointID:endpointID]];
}

- (BOOL)_deleteClusterIndexForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID
{
    dispatch_assert_queue(_storageDelegateQueue);

    if (!nodeID || !endpointID) {
        MTR_LOG_ERROR("%s: unexpected nil input", __func__);
        return NO;
    }

    return [self _removeAttributeCacheValueForKey:[self _clusterIndexKeyForNodeID:nodeID endpointID:endpointID]];
}

static NSString * sAttributeCacheClusterDataKeyPrefix = @"attrCacheClusterData";

- (NSString *)_clusterDataKeyForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID
{
    return [sAttributeCacheClusterDataKeyPrefix stringByAppendingFormat:@":0x%016llX:0x%04X:0x%08lX", nodeID.unsignedLongLongValue, endpointID.unsignedShortValue, clusterID.unsignedLongValue];
}

- (nullable MTRDeviceClusterData *)_fetchClusterDataForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID
{
    dispatch_assert_queue(_storageDelegateQueue);

    if (!nodeID || !endpointID || !clusterID) {
        MTR_LOG_ERROR("%s: unexpected nil input", __func__);
        return nil;
    }

    return [self _fetchAttributeCacheValueForKey:[self _clusterDataKeyForNodeID:nodeID endpointID:endpointID clusterID:clusterID] expectedClass:[MTRDeviceClusterData class]];
}

- (BOOL)_storeClusterData:(MTRDeviceClusterData *)clusterData forNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID
{
    dispatch_assert_queue(_storageDelegateQueue);

    if (!nodeID || !endpointID || !clusterID || !clusterData) {
        MTR_LOG_ERROR("%s: unexpected nil input", __func__);
        return NO;
    }

    return [self _storeAttributeCacheValue:clusterData forKey:[self _clusterDataKeyForNodeID:nodeID endpointID:endpointID clusterID:clusterID]];
}

- (BOOL)_deleteClusterDataForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID
{
    dispatch_assert_queue(_storageDelegateQueue);

    if (!nodeID || !endpointID || !clusterID) {
        MTR_LOG_ERROR("%s: unexpected nil input", __func__);
        return NO;
    }

    return [self _removeAttributeCacheValueForKey:[self _clusterDataKeyForNodeID:nodeID endpointID:endpointID clusterID:clusterID]];
}

#pragma - Attribute Cache management

#ifndef ATTRIBUTE_CACHE_VERBOSE_LOGGING
#define ATTRIBUTE_CACHE_VERBOSE_LOGGING 0
#endif

#ifdef DEBUG
- (void)unitTestPruneEmptyStoredClusterDataBranches
{
    dispatch_sync(_storageDelegateQueue, ^{
        [self _pruneEmptyStoredClusterDataBranches];
    });
}
#endif

- (void)_pruneEmptyStoredClusterDataBranches
{
    dispatch_assert_queue(_storageDelegateQueue);

    NSUInteger storeFailures = 0;

    // Fetch node index
    NSArray<NSNumber *> * nodeIndex = [self _fetchNodeIndex];
    NSMutableArray<NSNumber *> * nodeIndexCopy = [nodeIndex mutableCopy];

    for (NSNumber * nodeID in nodeIndex) {
        // Fetch endpoint index
        NSArray<NSNumber *> * endpointIndex = [self _fetchEndpointIndexForNodeID:nodeID];
        NSMutableArray<NSNumber *> * endpointIndexCopy = [endpointIndex mutableCopy];

        for (NSNumber * endpointID in endpointIndex) {
            // Fetch cluster index
            NSArray<NSNumber *> * clusterIndex = [self _fetchClusterIndexForNodeID:nodeID endpointID:endpointID];
            NSMutableArray<NSNumber *> * clusterIndexCopy = [clusterIndex mutableCopy];

            for (NSNumber * clusterID in clusterIndex) {
                // Fetch cluster data, if it exists.
                MTRDeviceClusterData * clusterData = [self _fetchClusterDataForNodeID:nodeID endpointID:endpointID clusterID:clusterID];
                if (!clusterData) {
                    [clusterIndexCopy removeObject:clusterID];
                }
            }

            if (clusterIndex.count != clusterIndexCopy.count) {
                BOOL success;
                if (clusterIndexCopy.count) {
                    success = [self _storeClusterIndex:clusterIndexCopy forNodeID:nodeID endpointID:endpointID];
                } else {
                    [endpointIndexCopy removeObject:endpointID];
                    success = [self _deleteClusterIndexForNodeID:nodeID endpointID:endpointID];
                }
                if (!success) {
                    storeFailures++;
                    MTR_LOG_ERROR("Store failed in _pruneEmptyStoredClusterDataBranches for clusterIndex (%lu) @ node 0x%016llX endpoint %u", static_cast<unsigned long>(clusterIndexCopy.count), nodeID.unsignedLongLongValue, endpointID.unsignedShortValue);
                }
            }
        }

        if (endpointIndex.count != endpointIndexCopy.count) {
            BOOL success;
            if (endpointIndexCopy.count) {
                success = [self _storeEndpointIndex:endpointIndexCopy forNodeID:nodeID];
            } else {
                [nodeIndexCopy removeObject:nodeID];
                success = [self _deleteEndpointIndexForNodeID:nodeID];
            }
            if (!success) {
                storeFailures++;
                MTR_LOG_ERROR("Store failed in _pruneEmptyStoredClusterDataBranches for endpointIndex (%lu) @ node 0x%016llX", static_cast<unsigned long>(endpointIndexCopy.count), nodeID.unsignedLongLongValue);
            }
        }
    }

    if (nodeIndex.count != nodeIndexCopy.count) {
        BOOL success;
        if (nodeIndexCopy.count) {
            success = [self _storeNodeIndex:nodeIndexCopy];
        } else {
            success = [self _deleteNodeIndex];
        }
        if (!success) {
            storeFailures++;
            MTR_LOG_ERROR("Store failed in _pruneEmptyStoredClusterDataBranches for nodeIndex (%lu)", static_cast<unsigned long>(nodeIndexCopy.count));
        }
    }

    if (storeFailures) {
        MTR_LOG_ERROR("Store failed in _pruneEmptyStoredClusterDataBranches: failure count %lu", static_cast<unsigned long>(storeFailures));
    }
}

- (void)_clearStoredClusterDataForNodeID:(NSNumber *)nodeID
{
    dispatch_assert_queue(_storageDelegateQueue);

    NSUInteger endpointsClearAttempts = 0;
    NSUInteger clusterDataClearAttempts = 0;
    NSUInteger endpointsCleared = 0;
    NSUInteger clusterDataCleared = 0;

    // Fetch endpoint index
    NSArray<NSNumber *> * endpointIndex = [self _fetchEndpointIndexForNodeID:nodeID];

    endpointsClearAttempts += endpointIndex.count;
    for (NSNumber * endpointID in endpointIndex) {
        // Fetch cluster index
        NSArray<NSNumber *> * clusterIndex = [self _fetchClusterIndexForNodeID:nodeID endpointID:endpointID];

        clusterDataClearAttempts += clusterIndex.count; // Assuming every cluster has cluster data
        for (NSNumber * clusterID in clusterIndex) {
            BOOL success = [self _deleteClusterDataForNodeID:nodeID endpointID:endpointID clusterID:clusterID];
            if (!success) {
                MTR_LOG_ERROR("Delete failed for clusterData @ node 0x%016llX endpoint %u cluster 0x%08lX", nodeID.unsignedLongLongValue, endpointID.unsignedShortValue, clusterID.unsignedLongValue);
            } else {
                clusterDataCleared++;
            }
        }

        BOOL success = [self _deleteClusterIndexForNodeID:nodeID endpointID:endpointID];
        if (!success) {
            MTR_LOG_ERROR("Delete failed for clusterIndex @ node 0x%016llX endpoint %u", nodeID.unsignedLongLongValue, endpointID.unsignedShortValue);
        } else {
            endpointsCleared++;
        }
    }

    BOOL success = [self _deleteEndpointIndexForNodeID:nodeID];
    if (!success) {
        MTR_LOG_ERROR("Delete failed for endpointIndex @ node 0x%016llX", nodeID.unsignedLongLongValue);
    }

    MTR_LOG("clearStoredClusterDataForNodeID: deleted endpoints %lu/%lu clusters %lu/%lu", static_cast<unsigned long>(endpointsCleared), static_cast<unsigned long>(endpointsClearAttempts), static_cast<unsigned long>(clusterDataCleared), static_cast<unsigned long>(clusterDataClearAttempts));
}

- (void)clearStoredClusterDataForNodeID:(NSNumber *)nodeID
{
    dispatch_async(_storageDelegateQueue, ^{
        [self _clearStoredClusterDataForNodeID:nodeID];
        NSArray<NSNumber *> * nodeIndex = [self _fetchNodeIndex];
        NSMutableArray<NSNumber *> * nodeIndexCopy = [nodeIndex mutableCopy];
        [nodeIndexCopy removeObject:nodeID];
        if (nodeIndex.count != nodeIndexCopy.count) {
            BOOL success;
            if (nodeIndexCopy.count) {
                success = [self _storeNodeIndex:nodeIndexCopy];
            } else {
                success = [self _deleteNodeIndex];
            }
            if (!success) {
                MTR_LOG_ERROR("Store failed in clearStoredAttributesForNodeID for nodeIndex (%lu)", static_cast<unsigned long>(nodeIndexCopy.count));
            }
        }
    });
}

- (void)clearAllStoredClusterData
{
    dispatch_async(_storageDelegateQueue, ^{
        // Fetch node index
        NSArray<NSNumber *> * nodeIndex = [self _fetchNodeIndex];

        for (NSNumber * nodeID in nodeIndex) {
            [self _clearStoredClusterDataForNodeID:nodeID];
        }

        BOOL success = [self _deleteNodeIndex];
        if (!success) {
            MTR_LOG_ERROR("Delete failed for nodeIndex");
        }
    });
}

- (nullable NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *)getStoredClusterDataForNodeID:(NSNumber *)nodeID
{
    if (!nodeID) {
        MTR_LOG_ERROR("%s: unexpected nil input", __func__);
        return nil;
    }

    __block NSMutableDictionary<MTRClusterPath *, MTRDeviceClusterData *> * clusterDataToReturn = nil;
    dispatch_sync(_storageDelegateQueue, ^{
        // Fetch node index
        NSArray<NSNumber *> * nodeIndex = [self _fetchNodeIndex];

#if ATTRIBUTE_CACHE_VERBOSE_LOGGING
        MTR_LOG("Fetch got %lu values for nodeIndex", static_cast<unsigned long>(nodeIndex.count));
#endif

        if (![nodeIndex containsObject:nodeID]) {
            // Sanity check and delete if nodeID exists in index
            NSArray<NSNumber *> * endpointIndex = [self _fetchEndpointIndexForNodeID:nodeID];
            if (endpointIndex) {
                MTR_LOG_ERROR("Persistent attribute cache contains orphaned entry for nodeID %@ - deleting", nodeID);
                // _clearStoredClusterDataForNodeID because we are are already
                // on the _storageDelegateQueue and do not need to modify the
                // node index in this case.
                [self _clearStoredClusterDataForNodeID:nodeID];
            }

            MTR_LOG("Fetch got no value for endpointIndex @ node 0x%016llX", nodeID.unsignedLongLongValue);
            clusterDataToReturn = nil;
            return;
        }

        // Fetch endpoint index
        NSArray<NSNumber *> * endpointIndex = [self _fetchEndpointIndexForNodeID:nodeID];

#if ATTRIBUTE_CACHE_VERBOSE_LOGGING
        MTR_LOG("Fetch got %lu values for endpointIndex @ node 0x%016llX", static_cast<unsigned long>(endpointIndex.count), nodeID.unsignedLongLongValue);
#endif

        for (NSNumber * endpointID in endpointIndex) {
            // Fetch cluster index
            NSArray<NSNumber *> * clusterIndex = [self _fetchClusterIndexForNodeID:nodeID endpointID:endpointID];

#if ATTRIBUTE_CACHE_VERBOSE_LOGGING
            MTR_LOG("Fetch got %lu values for clusterIndex @ node 0x%016llX %u", static_cast<unsigned long>(clusterIndex.count), nodeID.unsignedLongLongValue, endpointID.unsignedShortValue);
#endif

            for (NSNumber * clusterID in clusterIndex) {
                // Fetch cluster data
                MTRDeviceClusterData * clusterData = [self _fetchClusterDataForNodeID:nodeID endpointID:endpointID clusterID:clusterID];
                if (!clusterData) {
                    MTR_LOG("Fetch got no value for clusterData @ node 0x%016llX endpoint %u cluster 0x%08lX", nodeID.unsignedLongLongValue, endpointID.unsignedShortValue, clusterID.unsignedLongValue);
                    continue;
                }

#if ATTRIBUTE_CACHE_VERBOSE_LOGGING
                MTR_LOG("Fetch got clusterData @ node 0x%016llX endpoint %u cluster 0x%08lX", nodeID.unsignedLongLongValue, endpointID.unsignedShortValue, clusterID.unsignedLongValue);
#endif

                MTRClusterPath * path = [MTRClusterPath clusterPathWithEndpointID:endpointID clusterID:clusterID];
                if (!clusterDataToReturn) {
                    clusterDataToReturn = [NSMutableDictionary dictionary];
                }
                clusterDataToReturn[path] = clusterData;
            }
        }
    });

    return clusterDataToReturn;
}

- (nullable MTRDeviceClusterData *)getStoredClusterDataForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID
{
    // Don't bother checking our indices here, since this will only be called
    // when the consumer knows that we're supposed to have data for this cluster
    // path.
    __block MTRDeviceClusterData * clusterDataToReturn = nil;
    dispatch_sync(_storageDelegateQueue, ^{
        clusterDataToReturn = [self _fetchClusterDataForNodeID:nodeID endpointID:endpointID clusterID:clusterID];
    });
    return clusterDataToReturn;
}

// Utility for constructing dictionary of nodeID to cluster data from dictionary of storage keys
- (nullable NSDictionary<NSNumber *, NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *> *)_getClusterDataFromSecureLocalValues:(NSDictionary<NSString *, id> *)secureLocalValues
{
    NSMutableDictionary<NSNumber *, NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *> * clusterDataByNodeToReturn = nil;

    if (![secureLocalValues isKindOfClass:[NSDictionary class]]) {
        return nil;
    }

    // Fetch node index
    NSArray<NSNumber *> * nodeIndex = secureLocalValues[sAttributeCacheNodeIndexKey];

    if (![nodeIndex isKindOfClass:[NSArray class]]) {
        return nil;
    }

    for (NSNumber * nodeID in nodeIndex) {
        if (![nodeID isKindOfClass:[NSNumber class]]) {
            continue;
        }

        NSMutableDictionary<MTRClusterPath *, MTRDeviceClusterData *> * clusterDataForNode = nil;
        NSArray<NSNumber *> * endpointIndex = secureLocalValues[[self _endpointIndexKeyForNodeID:nodeID]];

        if (![endpointIndex isKindOfClass:[NSArray class]]) {
            continue;
        }

        for (NSNumber * endpointID in endpointIndex) {
            if (![endpointID isKindOfClass:[NSNumber class]]) {
                continue;
            }

            NSArray<NSNumber *> * clusterIndex = secureLocalValues[[self _clusterIndexKeyForNodeID:nodeID endpointID:endpointID]];

            if (![clusterIndex isKindOfClass:[NSArray class]]) {
                continue;
            }

            for (NSNumber * clusterID in clusterIndex) {
                if (![clusterID isKindOfClass:[NSNumber class]]) {
                    continue;
                }

                MTRDeviceClusterData * clusterData = secureLocalValues[[self _clusterDataKeyForNodeID:nodeID endpointID:endpointID clusterID:clusterID]];
                if (!clusterData) {
                    continue;
                }
                if (![clusterData isKindOfClass:[MTRDeviceClusterData class]]) {
                    continue;
                }
                MTRClusterPath * clusterPath = [MTRClusterPath clusterPathWithEndpointID:endpointID clusterID:clusterID];
                if (!clusterDataForNode) {
                    clusterDataForNode = [NSMutableDictionary dictionary];
                }
                clusterDataForNode[clusterPath] = clusterData;
            }
        }

        if (clusterDataForNode.count) {
            if (!clusterDataByNodeToReturn) {
                clusterDataByNodeToReturn = [NSMutableDictionary dictionary];
            }
            clusterDataByNodeToReturn[nodeID] = clusterDataForNode;
        }
    }

    return clusterDataByNodeToReturn;
}

- (void)storeClusterData:(NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *)clusterData forNodeID:(NSNumber *)nodeID
{
    if (!nodeID) {
        MTR_LOG_ERROR("%s: unexpected nil input", __func__);
        return;
    }

    if (!clusterData.count) {
        MTR_LOG_ERROR("%s: nothing to store", __func__);
        return;
    }

    dispatch_async(_storageDelegateQueue, ^{
        NSUInteger storeFailures = 0;

        NSMutableDictionary<NSString *, id<NSSecureCoding>> * bulkValuesToStore = nil;
        if ([self->_storageDelegate respondsToSelector:@selector(controller:storeValues:securityLevel:sharingType:)]) {
            bulkValuesToStore = [NSMutableDictionary dictionary];
        }

        // A map of endpoint => list of clusters modified for that endpoint so cluster indexes can be updated later
        NSMutableDictionary<NSNumber *, NSMutableSet<NSNumber *> *> * clustersModified = [NSMutableDictionary dictionary];

        // Write cluster specific data first
        for (MTRClusterPath * path in clusterData) {
            MTRDeviceClusterData * data = clusterData[path];
#if ATTRIBUTE_CACHE_VERBOSE_LOGGING
            MTR_LOG("Attempt to store clusterData @ node 0x%016llX endpoint %u cluster 0x%08lX", nodeID.unsignedLongLongValue, path.endpoint.unsignedShortValue, path.cluster.unsignedLongValue);
#endif

            if (bulkValuesToStore) {
                bulkValuesToStore[[self _clusterDataKeyForNodeID:nodeID endpointID:path.endpoint clusterID:path.cluster]] = data;
            } else {
                // Store cluster data
                BOOL storeFailed = ![self _storeClusterData:data forNodeID:nodeID endpointID:path.endpoint clusterID:path.cluster];
                if (storeFailed) {
                    storeFailures++;
                    MTR_LOG_ERROR("Store failed for clusterData @ node 0x%016llX endpoint %u cluster 0x%08lX", nodeID.unsignedLongLongValue, path.endpoint.unsignedShortValue, path.cluster.unsignedLongValue);
                }
            }

            // Note the cluster as modified for the endpoint
            NSMutableSet<NSNumber *> * clustersInEndpoint = clustersModified[path.endpoint];
            if (!clustersInEndpoint) {
                clustersInEndpoint = [NSMutableSet set];
                clustersModified[path.endpoint] = clustersInEndpoint;
            }
            [clustersInEndpoint addObject:path.cluster];
        }

        // Prepare to tally all endpoints touched
        NSArray<NSNumber *> * endpointIndex = [self _fetchEndpointIndexForNodeID:nodeID];
        BOOL endpointIndexModified = NO;
        NSMutableArray<NSNumber *> * endpointIndexToStore;
        if (endpointIndex) {
            endpointIndexToStore = [endpointIndex mutableCopy];
        } else {
            endpointIndexToStore = [NSMutableArray array];
            endpointIndexModified = YES;
        }

        for (NSNumber * endpointID in clustersModified) {
            if (![endpointIndexToStore containsObject:endpointID]) {
                [endpointIndexToStore addObject:endpointID];
                endpointIndexModified = YES;
            }

            // Get cluster index from storage and prepare to tally clusters touched
            NSSet * newClusters = clustersModified[endpointID];
            NSArray<NSNumber *> * clusterIndex = [self _fetchClusterIndexForNodeID:nodeID endpointID:endpointID];
            BOOL clusterIndexModified = NO;
            NSMutableArray<NSNumber *> * clusterIndexToStore;
            if (clusterIndex) {
                clusterIndexToStore = [clusterIndex mutableCopy];
            } else {
                clusterIndexToStore = [NSMutableArray array];
                clusterIndexModified = YES;
            }

            for (NSNumber * clusterID in newClusters) {
                if (![clusterIndexToStore containsObject:clusterID]) {
                    [clusterIndexToStore addObject:clusterID];
                    clusterIndexModified = YES;
                }
            }

            if (clusterIndexModified) {
                if (bulkValuesToStore) {
                    bulkValuesToStore[[self _clusterIndexKeyForNodeID:nodeID endpointID:endpointID]] = clusterIndexToStore;
                } else {
                    BOOL storeFailed = ![self _storeClusterIndex:clusterIndexToStore forNodeID:nodeID endpointID:endpointID];
                    if (storeFailed) {
                        storeFailures++;
                        MTR_LOG_ERROR("Store failed for clusterIndex @ node 0x%016llX endpoint %u", nodeID.unsignedLongLongValue, endpointID.unsignedShortValue);
                        continue;
                    }
                }
            }
        }

        // Update endpoint index as needed
        if (endpointIndexModified) {
            if (bulkValuesToStore) {
                bulkValuesToStore[[self _endpointIndexKeyForNodeID:nodeID]] = endpointIndexToStore;
            } else {
                BOOL storeFailed = ![self _storeEndpointIndex:endpointIndexToStore forNodeID:nodeID];
                if (storeFailed) {
                    storeFailures++;
                    MTR_LOG_ERROR("Store failed for endpointIndex @ node 0x%016llX", nodeID.unsignedLongLongValue);
                }
            }
        }

        // Check if node index needs updating / creation
        NSArray<NSNumber *> * nodeIndex = [self _fetchNodeIndex];
        NSArray<NSNumber *> * nodeIndexToStore = nil;
        if (!nodeIndex) {
            // Ensure node index exists
            nodeIndexToStore = [NSArray arrayWithObject:nodeID];
        } else if (![nodeIndex containsObject:nodeID]) {
            nodeIndexToStore = [nodeIndex arrayByAddingObject:nodeID];
        }

        if (nodeIndexToStore) {
            if (bulkValuesToStore) {
                bulkValuesToStore[sAttributeCacheNodeIndexKey] = nodeIndexToStore;
            } else {
                BOOL storeFailed = ![self _storeNodeIndex:nodeIndexToStore];
                if (storeFailed) {
                    storeFailures++;
                    MTR_LOG_ERROR("Store failed for nodeIndex");
                }
            }
        }

        if (bulkValuesToStore) {
            BOOL storeFailed = ![self _bulkStoreAttributeCacheValues:bulkValuesToStore];
            if (storeFailed) {
                storeFailures++;
                MTR_LOG_ERROR("Store failed for bulk values count %lu", static_cast<unsigned long>(bulkValuesToStore.count));
            }
        }

        // In the rare event that store fails, allow all cluster data store attempts to go through and prune empty branches at the end altogether.
        if (storeFailures) {
            [self _pruneEmptyStoredClusterDataBranches];
            MTR_LOG_ERROR("Store failed in -storeClusterData:forNodeID: failure count %lu", static_cast<unsigned long>(storeFailures));
        }
    });
}

static NSString * sDeviceDataKeyPrefix = @"deviceData";

- (NSString *)_deviceDataKeyForNodeID:(NSNumber *)nodeID
{
    return [sDeviceDataKeyPrefix stringByAppendingFormat:@":0x%016llX", nodeID.unsignedLongLongValue];
}

- (nullable NSDictionary<NSString *, id> *)getStoredDeviceDataForNodeID:(NSNumber *)nodeID
{
    __block NSDictionary<NSString *, id> * deviceData = nil;
    dispatch_sync(_storageDelegateQueue, ^{
        MTRDeviceController * controller = self->_controller;
        VerifyOrReturn(controller != nil); // No way to call delegate without controller.

        id data;
        @autoreleasepool {
            data = [self->_storageDelegate controller:controller
                                          valueForKey:[self _deviceDataKeyForNodeID:nodeID]
                                        securityLevel:MTRStorageSecurityLevelSecure
                                          sharingType:MTRStorageSharingTypeNotShared];
        }
        if (data == nil) {
            return;
        }

        if (![data isKindOfClass:NSDictionary.class]) {
            return;
        }

        // Check that all the keys are in fact strings.
        NSDictionary * dictionary = data;
        for (id key in dictionary) {
            if (![key isKindOfClass:NSString.class]) {
                return;
            }
        }

        // We can't do value type verification; our API consumer will need
        // to do that.
        deviceData = dictionary;
    });
    return deviceData;
}

- (void)storeDeviceData:(NSDictionary<NSString *, id> *)data forNodeID:(NSNumber *)nodeID
{
    dispatch_async(_storageDelegateQueue, ^{
        MTRDeviceController * controller = self->_controller;
        VerifyOrReturn(controller != nil); // No way to call delegate without controller.

        // Ignore store failures, since they are not actionable for us here.
        [self->_storageDelegate controller:controller
                                storeValue:data
                                    forKey:[self _deviceDataKeyForNodeID:nodeID]
                             securityLevel:MTRStorageSecurityLevelSecure
                               sharingType:MTRStorageSharingTypeNotShared];
    });
}

@end

@implementation MTRCASESessionResumptionInfo

#pragma mark - NSSecureCoding

static NSString * const sNodeIDKey = @"nodeID";
static NSString * const sResumptionIDKey = @"resumptionID";
static NSString * const sSharedSecretKey = @"sharedSecret";
static NSString * const sCATsKey = @"CATs";

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (nullable instancetype)initWithCoder:(NSCoder *)decoder
{
    self = [super init];
    if (self == nil) {
        return nil;
    }

    _nodeID = [decoder decodeObjectOfClass:[NSNumber class] forKey:sNodeIDKey];
    // For some built-in classes decoder will decode to them even if we ask for a
    // different class (!).  So sanity-check what we got.
    if (_nodeID != nil && ![_nodeID isKindOfClass:[NSNumber class]]) {
        MTR_LOG_ERROR("MTRCASESessionResumptionInfo got %@ for node ID, not NSNumber.", _nodeID);
        return nil;
    }
    if (!IsValidNodeIDNumber(_nodeID)) {
        MTR_LOG_ERROR("MTRCASESessionResumptionInfo node ID has invalid value: %@", _nodeID);
        return nil;
    }

    _resumptionID = [decoder decodeObjectOfClass:[NSData class] forKey:sResumptionIDKey];
    if (_resumptionID != nil && ![_resumptionID isKindOfClass:[NSData class]]) {
        MTR_LOG_ERROR("MTRCASESessionResumptionInfo got %@ for resumption ID, not NSData.", _resumptionID);
        return nil;
    }

    _sharedSecret = [decoder decodeObjectOfClass:[NSData class] forKey:sSharedSecretKey];
    if (_sharedSecret != nil && ![_sharedSecret isKindOfClass:[NSData class]]) {
        MTR_LOG_ERROR("MTRCASESessionResumptionInfo got %@ for shared secret, not NSData.", _sharedSecret);
        return nil;
    }

    auto caseAuthenticatedTagArray = [decoder decodeArrayOfObjectsOfClass:[NSNumber class] forKey:sCATsKey];
    for (id value in caseAuthenticatedTagArray) {
        if (!IsValidCATNumber(value)) {
            MTR_LOG_ERROR("MTRCASESessionResumptionInfo CASE tag has invalid value: %@", value);
            return nil;
        }

        // Range-checking will be done when we try to convert the set to CATValues.
    }

    _caseAuthenticatedTags = [NSSet setWithArray:caseAuthenticatedTagArray];
    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:self.nodeID forKey:sNodeIDKey];
    [coder encodeObject:self.resumptionID forKey:sResumptionIDKey];
    [coder encodeObject:self.sharedSecret forKey:sSharedSecretKey];
    // Encode the CATs as an array, so that we can decodeArrayOfObjectsOfClass
    // to get type-safe decoding for them.
    [coder encodeObject:[self.caseAuthenticatedTags allObjects] forKey:sCATsKey];
}

@end

NSSet<Class> * MTRDeviceControllerStorageClasses()
{
    // This only needs to return the classes for toplevel things we are storing,
    // plus NSNumber because some archivers use that internally to store
    // information about what's being archived.
    static NSSet * const sStorageClasses = [NSSet setWithArray:@[
        [NSNumber class],
        [NSData class],
        [NSArray class],
        [MTRCASESessionResumptionInfo class],
        [NSDictionary class],
        [NSString class],
        [MTRAttributePath class],
        [MTRDeviceClusterData class],
    ]];
    return sStorageClasses;
}
