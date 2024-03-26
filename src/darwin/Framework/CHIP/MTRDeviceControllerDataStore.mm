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
    MTRDeviceController * _controller;
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
            resumptionNodeList = [_storageDelegate controller:_controller
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
    auto * oldInfo = [self findResumptionInfoByNodeID:resumptionInfo.nodeID];
    dispatch_sync(_storageDelegateQueue, ^{
        if (oldInfo != nil) {
            // Remove old resumption id key.  No need to do that for the
            // node id, because we are about to overwrite it.
            [_storageDelegate controller:_controller
                       removeValueForKey:ResumptionByResumptionIDKey(oldInfo.resumptionID)
                           securityLevel:MTRStorageSecurityLevelSecure
                             sharingType:MTRStorageSharingTypeNotShared];
            [_nodesWithResumptionInfo removeObject:resumptionInfo.nodeID];
        }

        [_storageDelegate controller:_controller
                          storeValue:resumptionInfo
                              forKey:ResumptionByNodeIDKey(resumptionInfo.nodeID)
                       securityLevel:MTRStorageSecurityLevelSecure
                         sharingType:MTRStorageSharingTypeNotShared];
        [_storageDelegate controller:_controller
                          storeValue:resumptionInfo
                              forKey:ResumptionByResumptionIDKey(resumptionInfo.resumptionID)
                       securityLevel:MTRStorageSecurityLevelSecure
                         sharingType:MTRStorageSharingTypeNotShared];

        // Update our resumption info node list.
        [_nodesWithResumptionInfo addObject:resumptionInfo.nodeID];
        [_storageDelegate controller:_controller
                          storeValue:[_nodesWithResumptionInfo copy]
                              forKey:sResumptionNodeListKey
                       securityLevel:MTRStorageSecurityLevelSecure
                         sharingType:MTRStorageSharingTypeNotShared];
    });
}

- (void)clearAllResumptionInfo
{
    // Can we do less dispatch?  We would need to have a version of
    // _findResumptionInfoWithKey that assumes we are already on the right queue.
    for (NSNumber * nodeID in _nodesWithResumptionInfo) {
        auto * oldInfo = [self findResumptionInfoByNodeID:nodeID];
        if (oldInfo != nil) {
            dispatch_sync(_storageDelegateQueue, ^{
                [_storageDelegate controller:_controller
                           removeValueForKey:ResumptionByResumptionIDKey(oldInfo.resumptionID)
                               securityLevel:MTRStorageSecurityLevelSecure
                                 sharingType:MTRStorageSharingTypeNotShared];
                [_storageDelegate controller:_controller
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
    __block BOOL ok;
    dispatch_sync(_storageDelegateQueue, ^{
        ok = [_storageDelegate controller:_controller
                               storeValue:noc
                                   forKey:sLastLocallyUsedNOCKey
                            securityLevel:MTRStorageSecurityLevelSecure
                              sharingType:MTRStorageSharingTypeNotShared];
    });
    return ok ? CHIP_NO_ERROR : CHIP_ERROR_PERSISTED_STORAGE_FAILED;
}

- (MTRCertificateTLVBytes _Nullable)fetchLastLocallyUsedNOC
{
    __block id data;
    dispatch_sync(_storageDelegateQueue, ^{
        @autoreleasepool {
            data = [_storageDelegate controller:_controller
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
    // key could be nil if [NSString stringWithFormat] returns nil for some reason.
    if (key == nil) {
        return nil;
    }

    __block id resumptionInfo;
    dispatch_sync(_storageDelegateQueue, ^{
        @autoreleasepool {
            resumptionInfo = [_storageDelegate controller:_controller
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
 *        key: "attrCacheEndpointIndex:<nodeID>:endpointID"
 *        value: list of endpoint IDs
 *    ClusterID index
 *        key: "<nodeID+endpointID> clusters"
 *        value: list of cluster IDs
 *    AttributeID index
 *        key: "<nodeID+endpointID+clusterID> attributes"
 *        value: list of attribute IDs
 *    Attribute data entry:
 *        key: "<nodeID+endpointID+clusterID+attributeID> attribute data"
 *        value: serialized dictionary of attribute data
 *
 *    Attribute data dictionary
 *        Additional value "serial number"
 */

- (id)_fetchAttributeCacheValueForKey:(NSString *)key expectedClass:(Class)expectedClass;
{
    id data;
    @autoreleasepool {
        data = [_storageDelegate controller:_controller
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
    return [_storageDelegate controller:_controller
                             storeValue:value
                                 forKey:key
                          securityLevel:MTRStorageSecurityLevelSecure
                            sharingType:MTRStorageSharingTypeNotShared];
}

- (BOOL)_removeAttributeCacheValueForKey:(NSString *)key
{
    return [_storageDelegate controller:_controller
                      removeValueForKey:key
                          securityLevel:MTRStorageSecurityLevelSecure
                            sharingType:MTRStorageSharingTypeNotShared];
}

static NSString * sAttributeCacheNodeIndexKey = @"attrCacheNodeIndex";

- (nullable NSArray<NSNumber *> *)_fetchNodeIndex
{
    return [self _fetchAttributeCacheValueForKey:sAttributeCacheNodeIndexKey expectedClass:[NSArray class]];
}

- (BOOL)_storeNodeIndex:(NSArray<NSNumber *> *)nodeIndex
{
    return [self _storeAttributeCacheValue:nodeIndex forKey:sAttributeCacheNodeIndexKey];
}

- (BOOL)_deleteNodeIndex
{
    return [self _removeAttributeCacheValueForKey:sAttributeCacheNodeIndexKey];
}

static NSString * sAttributeCacheEndpointIndexKeyPrefix = @"attrCacheEndpointIndex";

- (NSString *)_endpointIndexKeyForNodeID:(NSNumber *)nodeID
{
    return [sAttributeCacheEndpointIndexKeyPrefix stringByAppendingFormat:@":0x%016llX", nodeID.unsignedLongLongValue];
}

- (nullable NSArray<NSNumber *> *)_fetchEndpointIndexForNodeID:(NSNumber *)nodeID
{
    return [self _fetchAttributeCacheValueForKey:[self _endpointIndexKeyForNodeID:nodeID] expectedClass:[NSArray class]];
}

- (BOOL)_storeEndpointIndex:(NSArray<NSNumber *> *)endpointIndex forNodeID:(NSNumber *)nodeID
{
    return [self _storeAttributeCacheValue:endpointIndex forKey:[self _endpointIndexKeyForNodeID:nodeID]];
}

- (BOOL)_deleteEndpointIndexForNodeID:(NSNumber *)nodeID
{
    return [self _removeAttributeCacheValueForKey:[self _endpointIndexKeyForNodeID:nodeID]];
}

static NSString * sAttributeCacheClusterIndexKeyPrefix = @"attrCacheClusterIndex";

- (NSString *)_clusterIndexKeyForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID
{
    return [sAttributeCacheClusterIndexKeyPrefix stringByAppendingFormat:@":0x%016llX:%0x04X", nodeID.unsignedLongLongValue, endpointID.unsignedShortValue];
}

- (nullable NSArray<NSNumber *> *)_fetchClusterIndexForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID
{
    return [self _fetchAttributeCacheValueForKey:[self _clusterIndexKeyForNodeID:nodeID endpointID:endpointID] expectedClass:[NSArray class]];
}

- (BOOL)_storeClusterIndex:(NSArray<NSNumber *> *)clusterIndex forNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID
{
    return [self _storeAttributeCacheValue:clusterIndex forKey:[self _clusterIndexKeyForNodeID:nodeID endpointID:endpointID]];
}

- (BOOL)_deleteClusterIndexForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID
{
    return [self _removeAttributeCacheValueForKey:[self _clusterIndexKeyForNodeID:nodeID endpointID:endpointID]];
}

static NSString * sAttributeCacheClusterDataKeyPrefix = @"attrCacheClusterData";

- (NSString *)_clusterDataKeyForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID
{
    return [sAttributeCacheClusterDataKeyPrefix stringByAppendingFormat:@":0x%016llX:%0x04X:0x%08lX", nodeID.unsignedLongLongValue, endpointID.unsignedShortValue, clusterID.unsignedLongValue];
}

- (nullable MTRDeviceClusterData *)_fetchClusterDataForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID
{
    return [self _fetchAttributeCacheValueForKey:[self _clusterDataKeyForNodeID:nodeID endpointID:endpointID clusterID:clusterID] expectedClass:[MTRDeviceClusterData class]];
}

- (BOOL)_storeClusterData:(MTRDeviceClusterData *)clusterData forNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID
{
    return [self _storeAttributeCacheValue:clusterData forKey:[self _clusterDataKeyForNodeID:nodeID endpointID:endpointID clusterID:clusterID]];
}

- (BOOL)_deleteClusterDataForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID
{
    return [self _removeAttributeCacheValueForKey:[self _clusterDataKeyForNodeID:nodeID endpointID:endpointID clusterID:clusterID]];
}

static NSString * sAttributeCacheAttributeIndexKeyPrefix = @"attrCacheAttributeIndex";

- (NSString *)_attributeIndexKeyForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID
{
    return [sAttributeCacheAttributeIndexKeyPrefix stringByAppendingFormat:@":0x%016llX:0x%04X:0x%08lX", nodeID.unsignedLongLongValue, endpointID.unsignedShortValue, clusterID.unsignedLongValue];
}

- (nullable NSArray<NSNumber *> *)_fetchAttributeIndexForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID
{
    return [self _fetchAttributeCacheValueForKey:[self _attributeIndexKeyForNodeID:nodeID endpointID:endpointID clusterID:clusterID] expectedClass:[NSArray class]];
}

- (BOOL)_storeAttributeIndex:(NSArray<NSNumber *> *)attributeIndex forNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID
{
    return [self _storeAttributeCacheValue:attributeIndex forKey:[self _attributeIndexKeyForNodeID:nodeID endpointID:endpointID clusterID:clusterID]];
}

- (BOOL)_deleteAttributeIndexForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID
{
    return [self _removeAttributeCacheValueForKey:[self _attributeIndexKeyForNodeID:nodeID endpointID:endpointID clusterID:clusterID]];
}

static NSString * sAttributeCacheAttributeValueKeyPrefix = @"attrCacheAttributeValue";

- (NSString *)_attributeValueKeyForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID attributeID:(NSNumber *)attributeID
{
    return [sAttributeCacheAttributeValueKeyPrefix stringByAppendingFormat:@":0x%016llX:0x%04X:0x%08lX:0x%08lX", nodeID.unsignedLongLongValue, endpointID.unsignedShortValue, clusterID.unsignedLongValue, attributeID.unsignedLongValue];
}

- (nullable NSDictionary *)_fetchAttributeValueForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID attributeID:(NSNumber *)attributeID
{
    return [self _fetchAttributeCacheValueForKey:[self _attributeValueKeyForNodeID:nodeID endpointID:endpointID clusterID:clusterID attributeID:attributeID] expectedClass:[NSDictionary class]];
}

- (BOOL)_storeAttributeValue:(NSDictionary *)value forNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID attributeID:(NSNumber *)attributeID
{
    return [self _storeAttributeCacheValue:value forKey:[self _attributeValueKeyForNodeID:nodeID endpointID:endpointID clusterID:clusterID attributeID:attributeID]];
}

- (BOOL)_deleteAttributeValueForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID attributeID:(NSNumber *)attributeID
{
    return [self _removeAttributeCacheValueForKey:[self _attributeValueKeyForNodeID:nodeID endpointID:endpointID clusterID:clusterID attributeID:attributeID]];
}

#pragma - Attribute Cache management

#ifndef ATTRIBUTE_CACHE_VERBOSE_LOGGING
#define ATTRIBUTE_CACHE_VERBOSE_LOGGING 0
#endif

- (nullable NSArray<NSDictionary *> *)getStoredAttributesForNodeID:(NSNumber *)nodeID
{
    __block NSMutableArray * attributesToReturn = nil;
    dispatch_sync(_storageDelegateQueue, ^{
        // Fetch node index
        NSArray<NSNumber *> * nodeIndex = [self _fetchNodeIndex];

#if ATTRIBUTE_CACHE_VERBOSE_LOGGING
        MTR_LOG_INFO("Fetch got %lu values for nodeIndex", static_cast<unsigned long>(nodeIndex.count));
#endif

        if (![nodeIndex containsObject:nodeID]) {
            // Sanity check and delete if nodeID exists in index
            NSArray<NSNumber *> * endpointIndex = [self _fetchEndpointIndexForNodeID:nodeID];
            if (endpointIndex) {
                MTR_LOG_ERROR("Persistent attribute cache contains orphaned entry for nodeID %@ - deleting", nodeID);
                [self clearStoredAttributesForNodeID:nodeID];
            }

            MTR_LOG_INFO("Fetch got no value for endpointIndex @ node 0x%016llX", nodeID.unsignedLongLongValue);
            attributesToReturn = nil;
            return;
        }

        // Fetch endpoint index
        NSArray<NSNumber *> * endpointIndex = [self _fetchEndpointIndexForNodeID:nodeID];

#if ATTRIBUTE_CACHE_VERBOSE_LOGGING
        MTR_LOG_INFO("Fetch got %lu values for endpointIndex @ node 0x%016llX", static_cast<unsigned long>(endpointIndex.count), nodeID.unsignedLongLongValue);
#endif

        for (NSNumber * endpointID in endpointIndex) {
            // Fetch endpoint index
            NSArray<NSNumber *> * clusterIndex = [self _fetchClusterIndexForNodeID:nodeID endpointID:endpointID];

#if ATTRIBUTE_CACHE_VERBOSE_LOGGING
            MTR_LOG_INFO("Fetch got %lu values for clusterIndex @ node 0x%016llX %u", static_cast<unsigned long>(clusterIndex.count), nodeID.unsignedLongLongValue, endpointID.unsignedShortValue);
#endif

            for (NSNumber * clusterID in clusterIndex) {
                // Fetch endpoint index
                NSArray<NSNumber *> * attributeIndex = [self _fetchAttributeIndexForNodeID:nodeID endpointID:endpointID clusterID:clusterID];

#if ATTRIBUTE_CACHE_VERBOSE_LOGGING
                MTR_LOG_INFO("Fetch got %lu values for attributeIndex @ node 0x%016llX endpoint %u cluster 0x%08lX", static_cast<unsigned long>(attributeIndex.count), nodeID.unsignedLongLongValue, endpointID.unsignedShortValue, clusterID.unsignedLongValue);
#endif

                for (NSNumber * attributeID in attributeIndex) {
                    NSDictionary * value = [self _fetchAttributeValueForNodeID:nodeID endpointID:endpointID clusterID:clusterID attributeID:attributeID];

#if ATTRIBUTE_CACHE_VERBOSE_LOGGING
                    MTR_LOG_INFO("Fetch got %u values for attribute value @ node 0x%016llX endpoint %u cluster 0x%08lX attribute 0x%08lX", value ? 1 : 0, nodeID.unsignedLongLongValue, endpointID.unsignedShortValue, clusterID.unsignedLongValue, attributeID.unsignedLongValue);
#endif

                    if (value) {
                        if (!attributesToReturn) {
                            attributesToReturn = [NSMutableArray array];
                        }

                        // Construct data-value dictionary and add to array
                        MTRAttributePath * path = [MTRAttributePath attributePathWithEndpointID:endpointID clusterID:clusterID attributeID:attributeID];
                        [attributesToReturn addObject:@ { MTRAttributePathKey : path, MTRDataKey : value }];
                    }
                }

                // TODO: Add per-cluster integrity check verification
            }
        }
    });

    return attributesToReturn;
}

#ifdef DEBUG
- (void)unitTestPruneEmptyStoredAttributesBranches
{
    dispatch_sync(_storageDelegateQueue, ^{
        [self _pruneEmptyStoredAttributesBranches];
    });
}
#endif

- (void)_pruneEmptyStoredAttributesBranches
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
            // Fetch endpoint index
            NSArray<NSNumber *> * clusterIndex = [self _fetchClusterIndexForNodeID:nodeID endpointID:endpointID];
            NSMutableArray<NSNumber *> * clusterIndexCopy = [clusterIndex mutableCopy];

            for (NSNumber * clusterID in clusterIndex) {
                // Fetch endpoint index
                NSArray<NSNumber *> * attributeIndex = [self _fetchAttributeIndexForNodeID:nodeID endpointID:endpointID clusterID:clusterID];
                NSMutableArray<NSNumber *> * attributeIndexCopy = [attributeIndex mutableCopy];

                for (NSNumber * attributeID in attributeIndex) {
                    NSDictionary * value = [self _fetchAttributeValueForNodeID:nodeID endpointID:endpointID clusterID:clusterID attributeID:attributeID];

                    if (!value) {
                        [attributeIndexCopy removeObject:attributeID];
                    }
                }

                if (attributeIndex.count != attributeIndexCopy.count) {
                    BOOL success;
                    BOOL clusterDataSuccess = YES;
                    if (attributeIndexCopy.count) {
                        success = [self _storeAttributeIndex:attributeIndexCopy forNodeID:nodeID endpointID:endpointID clusterID:clusterID];
                    } else {
                        [clusterIndexCopy removeObject:clusterID];
                        success = [self _deleteAttributeIndexForNodeID:nodeID endpointID:endpointID clusterID:clusterID];
                        clusterDataSuccess = [self _deleteClusterDataForNodeID:nodeID endpointID:endpointID clusterID:clusterID];
                    }
                    if (!success) {
                        storeFailures++;
                        MTR_LOG_INFO("Store failed in _pruneEmptyStoredAttributesBranches for attributeIndex (%lu) @ node 0x%016llX endpoint %u cluster 0x%08lX", static_cast<unsigned long>(attributeIndexCopy.count), nodeID.unsignedLongLongValue, endpointID.unsignedShortValue, clusterID.unsignedLongValue);
                    }
                    if (!clusterDataSuccess) {
                        storeFailures++;
                        MTR_LOG_INFO("Store failed in _pruneEmptyStoredAttributesBranches for clusterData @ node 0x%016llX endpoint %u cluster 0x%08lX", nodeID.unsignedLongLongValue, endpointID.unsignedShortValue, clusterID.unsignedLongValue);
                    }
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
                    MTR_LOG_INFO("Store failed in _pruneEmptyStoredAttributesBranches for clusterIndex (%lu) @ node 0x%016llX endpoint %u", static_cast<unsigned long>(clusterIndexCopy.count), nodeID.unsignedLongLongValue, endpointID.unsignedShortValue);
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
                MTR_LOG_INFO("Store failed in _pruneEmptyStoredAttributesBranches for endpointIndex (%lu) @ node 0x%016llX", static_cast<unsigned long>(endpointIndexCopy.count), nodeID.unsignedLongLongValue);
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
            MTR_LOG_INFO("Store failed in _pruneEmptyStoredAttributesBranches for nodeIndex (%lu)", static_cast<unsigned long>(nodeIndexCopy.count));
        }
    }

    if (storeFailures) {
        MTR_LOG_ERROR("Store failed in _pruneEmptyStoredAttributesBranches: failure count %lu", static_cast<unsigned long>(storeFailures));
    }
}

- (void)storeAttributeValues:(NSArray<NSDictionary *> *)dataValues forNodeID:(NSNumber *)nodeID
{
    dispatch_async(_storageDelegateQueue, ^{
        NSUInteger storeFailures = 0;

        for (NSDictionary * dataValue in dataValues) {
            MTRAttributePath * path = dataValue[MTRAttributePathKey];
            NSDictionary * value = dataValue[MTRDataKey];

#if ATTRIBUTE_CACHE_VERBOSE_LOGGING
            MTR_LOG_INFO("Attempt to store attribute value @ node 0x%016llX endpoint %u cluster 0x%08lX attribute 0x%08lX", nodeID.unsignedLongLongValue, path.endpoint.unsignedShortValue, path.cluster.unsignedLongValue, path.attribute.unsignedLongValue);
#endif

            BOOL storeFailed = NO;
            // Ensure node index exists
            NSArray<NSNumber *> * nodeIndex = [self _fetchNodeIndex];
            if (!nodeIndex) {
                nodeIndex = [NSArray arrayWithObject:nodeID];
                storeFailed = ![self _storeNodeIndex:nodeIndex];
            } else if (![nodeIndex containsObject:nodeID]) {
                storeFailed = ![self _storeNodeIndex:[nodeIndex arrayByAddingObject:nodeID]];
            }
            if (storeFailed) {
                storeFailures++;
                MTR_LOG_INFO("Store failed for nodeIndex");
                continue;
            }

            // Ensure endpoint index exists
            NSArray<NSNumber *> * endpointIndex = [self _fetchEndpointIndexForNodeID:nodeID];
            if (!endpointIndex) {
                endpointIndex = [NSArray arrayWithObject:path.endpoint];
                storeFailed = ![self _storeEndpointIndex:endpointIndex forNodeID:nodeID];
            } else if (![endpointIndex containsObject:path.endpoint]) {
                storeFailed = ![self _storeEndpointIndex:[endpointIndex arrayByAddingObject:path.endpoint] forNodeID:nodeID];
            }
            if (storeFailed) {
                storeFailures++;
                MTR_LOG_INFO("Store failed for endpointIndex @ node 0x%016llX", nodeID.unsignedLongLongValue);
                continue;
            }

            // Ensure cluster index exists
            NSArray<NSNumber *> * clusterIndex = [self _fetchClusterIndexForNodeID:nodeID endpointID:path.endpoint];
            if (!clusterIndex) {
                clusterIndex = [NSArray arrayWithObject:path.cluster];
                storeFailed = ![self _storeClusterIndex:clusterIndex forNodeID:nodeID endpointID:path.endpoint];
            } else if (![clusterIndex containsObject:path.cluster]) {
                storeFailed = ![self _storeClusterIndex:[clusterIndex arrayByAddingObject:path.cluster] forNodeID:nodeID endpointID:path.endpoint];
            }
            if (storeFailed) {
                storeFailures++;
                MTR_LOG_INFO("Store failed for clusterIndex @ node 0x%016llX endpoint %u", nodeID.unsignedLongLongValue, path.endpoint.unsignedShortValue);
                continue;
            }

            // TODO: Add per-cluster integrity check calculation and store with cluster
            // TODO: Think about adding more integrity check for endpoint and node levels as well

            // Ensure attribute index exists
            NSArray<NSNumber *> * attributeIndex = [self _fetchAttributeIndexForNodeID:nodeID endpointID:path.endpoint clusterID:path.cluster];
            if (!attributeIndex) {
                attributeIndex = [NSArray arrayWithObject:path.attribute];
                storeFailed = ![self _storeAttributeIndex:attributeIndex forNodeID:nodeID endpointID:path.endpoint clusterID:path.cluster];
            } else if (![attributeIndex containsObject:path.attribute]) {
                storeFailed = ![self _storeAttributeIndex:[attributeIndex arrayByAddingObject:path.attribute] forNodeID:nodeID endpointID:path.endpoint clusterID:path.cluster];
            }
            if (storeFailed) {
                storeFailures++;
                MTR_LOG_INFO("Store failed for attributeIndex @ node 0x%016llX endpoint %u cluster 0x%08lX", nodeID.unsignedLongLongValue, path.endpoint.unsignedShortValue, path.cluster.unsignedLongValue);
                continue;
            }

            // Store value
            storeFailed = ![self _storeAttributeValue:value forNodeID:nodeID endpointID:path.endpoint clusterID:path.cluster attributeID:path.attribute];
            if (storeFailed) {
                storeFailures++;
                MTR_LOG_INFO("Store failed for attribute value @ node 0x%016llX endpoint %u cluster 0x%08lX attribute 0x%08lX", nodeID.unsignedLongLongValue, path.endpoint.unsignedShortValue, path.cluster.unsignedLongValue, path.attribute.unsignedLongValue);
            }
        }

        // In the rare event that store fails, allow all attribute store attempts to go through and prune empty branches at the end altogether.
        if (storeFailures) {
            [self _pruneEmptyStoredAttributesBranches];
            MTR_LOG_ERROR("Store failed in -storeAttributeValues:forNodeID: failure count %lu", static_cast<unsigned long>(storeFailures));
        }
    });
}

- (void)_clearStoredAttributesForNodeID:(NSNumber *)nodeID
{
    NSUInteger endpointsClearAttempts = 0;
    NSUInteger clustersClearAttempts = 0;
    NSUInteger clusterDataClearAttempts = 0;
    NSUInteger attributesClearAttempts = 0;
    NSUInteger endpointsCleared = 0;
    NSUInteger clustersCleared = 0;
    NSUInteger clusterDataCleared = 0;
    NSUInteger attributesCleared = 0;

    // Fetch endpoint index
    NSArray<NSNumber *> * endpointIndex = [self _fetchEndpointIndexForNodeID:nodeID];

    endpointsClearAttempts += endpointIndex.count;
    for (NSNumber * endpointID in endpointIndex) {
        // Fetch cluster index
        NSArray<NSNumber *> * clusterIndex = [self _fetchClusterIndexForNodeID:nodeID endpointID:endpointID];

        clustersClearAttempts += clusterIndex.count;
        clusterDataClearAttempts += clusterIndex.count; // Assuming every cluster has cluster data
        for (NSNumber * clusterID in clusterIndex) {
            // Fetch attribute index
            NSArray<NSNumber *> * attributeIndex = [self _fetchAttributeIndexForNodeID:nodeID endpointID:endpointID clusterID:clusterID];

            attributesClearAttempts += attributeIndex.count;
            for (NSNumber * attributeID in attributeIndex) {
                BOOL success = [self _deleteAttributeValueForNodeID:nodeID endpointID:endpointID clusterID:clusterID attributeID:attributeID];
                if (!success) {
                    MTR_LOG_INFO("Delete failed for attribute value @ node 0x%016llX endpoint %u cluster 0x%08lX attribute 0x%08lX", nodeID.unsignedLongLongValue, endpointID.unsignedShortValue, clusterID.unsignedLongValue, attributeID.unsignedLongValue);
                } else {
                    attributesCleared++;
                }
            }

            BOOL success = [self _deleteAttributeIndexForNodeID:nodeID endpointID:endpointID clusterID:clusterID];
            if (!success) {
                MTR_LOG_INFO("Delete failed for attributeIndex @ node 0x%016llX endpoint %u cluster 0x%08lX", nodeID.unsignedLongLongValue, endpointID.unsignedShortValue, clusterID.unsignedLongValue);
            } else {
                clustersCleared++;
            }

            success = [self _deleteClusterDataForNodeID:nodeID endpointID:endpointID clusterID:clusterID];
            if (!success) {
                MTR_LOG_INFO("Delete failed for clusterData @ node 0x%016llX endpoint %u cluster 0x%08lX", nodeID.unsignedLongLongValue, endpointID.unsignedShortValue, clusterID.unsignedLongValue);
            } else {
                clusterDataCleared++;
            }
        }

        BOOL success = [self _deleteClusterIndexForNodeID:nodeID endpointID:endpointID];
        if (!success) {
            MTR_LOG_INFO("Delete failed for clusterIndex @ node 0x%016llX endpoint %u", nodeID.unsignedLongLongValue, endpointID.unsignedShortValue);
        } else {
            endpointsCleared++;
        }
    }

    BOOL success = [self _deleteEndpointIndexForNodeID:nodeID];
    if (!success) {
        MTR_LOG_INFO("Delete failed for endpointrIndex @ node 0x%016llX", nodeID.unsignedLongLongValue);
    }

    MTR_LOG_INFO("clearStoredAttributesForNodeID: deleted endpoints %lu/%lu clusters %lu/%lu clusterData %lu/%lu attributes %lu/%lu", static_cast<unsigned long>(endpointsCleared), static_cast<unsigned long>(endpointsClearAttempts), static_cast<unsigned long>(clustersCleared), static_cast<unsigned long>(clustersClearAttempts), static_cast<unsigned long>(clusterDataCleared), static_cast<unsigned long>(clusterDataClearAttempts), static_cast<unsigned long>(attributesCleared), static_cast<unsigned long>(attributesClearAttempts));
}

- (void)clearStoredAttributesForNodeID:(NSNumber *)nodeID
{
    dispatch_async(_storageDelegateQueue, ^{
        [self _clearStoredAttributesForNodeID:nodeID];
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
                MTR_LOG_INFO("Store failed in clearStoredAttributesForNodeID for nodeIndex (%lu)", static_cast<unsigned long>(nodeIndexCopy.count));
            }
        }
    });
}

- (void)clearAllStoredAttributes
{
    dispatch_async(_storageDelegateQueue, ^{
        // Fetch node index
        NSArray<NSNumber *> * nodeIndex = [self _fetchNodeIndex];

        for (NSNumber * nodeID in nodeIndex) {
            [self _clearStoredAttributesForNodeID:nodeID];
        }

        BOOL success = [self _deleteNodeIndex];
        if (!success) {
            MTR_LOG_INFO("Delete failed for nodeIndex");
        }
    });
}

- (nullable NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *)getStoredClusterDataForNodeID:(NSNumber *)nodeID
{
    __block NSMutableDictionary<MTRClusterPath *, MTRDeviceClusterData *> * clusterDataToReturn = nil;
    dispatch_sync(_storageDelegateQueue, ^{
        // Fetch node index
        NSArray<NSNumber *> * nodeIndex = [self _fetchNodeIndex];

#if ATTRIBUTE_CACHE_VERBOSE_LOGGING
        MTR_LOG_INFO("Fetch got %lu values for nodeIndex", static_cast<unsigned long>(nodeIndex.count));
#endif

        if (![nodeIndex containsObject:nodeID]) {
            // Sanity check and delete if nodeID exists in index
            NSArray<NSNumber *> * endpointIndex = [self _fetchEndpointIndexForNodeID:nodeID];
            if (endpointIndex) {
                MTR_LOG_ERROR("Persistent attribute cache contains orphaned entry for nodeID %@ - deleting", nodeID);
                [self clearStoredAttributesForNodeID:nodeID];
            }

            MTR_LOG_INFO("Fetch got no value for endpointIndex @ node 0x%016llX", nodeID.unsignedLongLongValue);
            clusterDataToReturn = nil;
            return;
        }

        // Fetch endpoint index
        NSArray<NSNumber *> * endpointIndex = [self _fetchEndpointIndexForNodeID:nodeID];

#if ATTRIBUTE_CACHE_VERBOSE_LOGGING
        MTR_LOG_INFO("Fetch got %lu values for endpointIndex @ node 0x%016llX", static_cast<unsigned long>(endpointIndex.count), nodeID.unsignedLongLongValue);
#endif

        for (NSNumber * endpointID in endpointIndex) {
            // Fetch endpoint index
            NSArray<NSNumber *> * clusterIndex = [self _fetchClusterIndexForNodeID:nodeID endpointID:endpointID];

#if ATTRIBUTE_CACHE_VERBOSE_LOGGING
            MTR_LOG_INFO("Fetch got %lu values for clusterIndex @ node 0x%016llX %u", static_cast<unsigned long>(clusterIndex.count), nodeID.unsignedLongLongValue, endpointID.unsignedShortValue);
#endif

            for (NSNumber * clusterID in clusterIndex) {
                // Fetch cluster data
                MTRDeviceClusterData * clusterData = [self _fetchClusterDataForNodeID:nodeID endpointID:endpointID clusterID:clusterID];
                if (!clusterData) {
                    MTR_LOG_INFO("Fetch got no value for clusterData @ node 0x%016llX endpoint %u cluster 0x%08lX", nodeID.unsignedLongLongValue, endpointID.unsignedShortValue, clusterID.unsignedLongValue);
                    continue;
                }

#if ATTRIBUTE_CACHE_VERBOSE_LOGGING
                MTR_LOG_INFO("Fetch got clusterData @ node 0x%016llX endpoint %u cluster 0x%08lX", nodeID.unsignedLongLongValue, endpointID.unsignedShortValue, clusterID.unsignedLongValue);
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

- (void)storeClusterData:(NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *)clusterData forNodeID:(NSNumber *)nodeID
{
    dispatch_async(_storageDelegateQueue, ^{
        NSUInteger storeFailures = 0;

        for (MTRClusterPath * path in clusterData) {
            MTRDeviceClusterData * data = clusterData[path];

#if ATTRIBUTE_CACHE_VERBOSE_LOGGING
            MTR_LOG_INFO("Attempt to store clusterData @ node 0x%016llX endpoint %u cluster 0x%08lX", nodeID.unsignedLongLongValue, path.endpoint.unsignedShortValue, path.cluster.unsignedLongValue);
#endif

            BOOL storeFailed = NO;
            // Ensure node index exists
            NSArray<NSNumber *> * nodeIndex = [self _fetchNodeIndex];
            if (!nodeIndex) {
                nodeIndex = [NSArray arrayWithObject:nodeID];
                storeFailed = ![self _storeNodeIndex:nodeIndex];
            } else if (![nodeIndex containsObject:nodeID]) {
                storeFailed = ![self _storeNodeIndex:[nodeIndex arrayByAddingObject:nodeID]];
            }
            if (storeFailed) {
                storeFailures++;
                MTR_LOG_INFO("Store failed for nodeIndex");
                continue;
            }

            // Ensure endpoint index exists
            NSArray<NSNumber *> * endpointIndex = [self _fetchEndpointIndexForNodeID:nodeID];
            if (!endpointIndex) {
                endpointIndex = [NSArray arrayWithObject:path.endpoint];
                storeFailed = ![self _storeEndpointIndex:endpointIndex forNodeID:nodeID];
            } else if (![endpointIndex containsObject:path.endpoint]) {
                storeFailed = ![self _storeEndpointIndex:[endpointIndex arrayByAddingObject:path.endpoint] forNodeID:nodeID];
            }
            if (storeFailed) {
                storeFailures++;
                MTR_LOG_INFO("Store failed for endpointIndex @ node 0x%016llX", nodeID.unsignedLongLongValue);
                continue;
            }

            // Ensure cluster index exists
            NSArray<NSNumber *> * clusterIndex = [self _fetchClusterIndexForNodeID:nodeID endpointID:path.endpoint];
            if (!clusterIndex) {
                clusterIndex = [NSArray arrayWithObject:path.cluster];
                storeFailed = ![self _storeClusterIndex:clusterIndex forNodeID:nodeID endpointID:path.endpoint];
            } else if (![clusterIndex containsObject:path.cluster]) {
                storeFailed = ![self _storeClusterIndex:[clusterIndex arrayByAddingObject:path.cluster] forNodeID:nodeID endpointID:path.endpoint];
            }
            if (storeFailed) {
                storeFailures++;
                MTR_LOG_INFO("Store failed for clusterIndex @ node 0x%016llX endpoint %u", nodeID.unsignedLongLongValue, path.endpoint.unsignedShortValue);
                continue;
            }

            // Store cluster data
            storeFailed = ![self _storeClusterData:data forNodeID:nodeID endpointID:path.endpoint clusterID:path.cluster];
            if (storeFailed) {
                storeFailures++;
                MTR_LOG_INFO("Store failed for clusterDAta @ node 0x%016llX endpoint %u cluster 0x%08lX", nodeID.unsignedLongLongValue, path.endpoint.unsignedShortValue, path.cluster.unsignedLongValue);
            }
        }

        // In the rare event that store fails, allow all attribute store attempts to go through and prune empty branches at the end altogether.
        if (storeFailures) {
            [self _pruneEmptyStoredAttributesBranches];
            MTR_LOG_ERROR("Store failed in -storeAttributeValues:forNodeID: failure count %lu", static_cast<unsigned long>(storeFailures));
        }
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
