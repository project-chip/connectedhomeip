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
        resumptionNodeList = [_storageDelegate controller:_controller
                                              valueForKey:sResumptionNodeListKey
                                            securityLevel:MTRStorageSecurityLevelSecure
                                              sharingType:MTRStorageSharingTypeNotShared];
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
        data = [_storageDelegate controller:_controller
                                valueForKey:sLastLocallyUsedNOCKey
                              securityLevel:MTRStorageSecurityLevelSecure
                                sharingType:MTRStorageSharingTypeNotShared];
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
        resumptionInfo = [_storageDelegate controller:_controller
                                          valueForKey:key
                                        securityLevel:MTRStorageSecurityLevelSecure
                                          sharingType:MTRStorageSharingTypeNotShared];
    });

    if (resumptionInfo == nil) {
        return nil;
    }

    if (![resumptionInfo isKindOfClass:[MTRCASESessionResumptionInfo class]]) {
        return nil;
    }

    return resumptionInfo;
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
    ]];
    return sStorageClasses;
}
