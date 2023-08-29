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

// TODO: FIXME: Figure out whether these are good key strings.
static NSString * sResumptionNodeListKey = @"caseResumptionNodeList";
static NSString * sLastUsedNOCKey = @"lastUsedControllerNOC";

static NSString * ResumptionByNodeIDKey(NSNumber * nodeID)
{
    return [NSString stringWithFormat:@"caseResumptionByNodeID/%llx", nodeID.unsignedLongLongValue];
}

static NSString * ResumptionByResumptionIDKey(NSData * resumptionID)
{
    return
        [NSString stringWithFormat:@"caseResumptionByResumptionID/%s", [resumptionID base64EncodedStringWithOptions:0].UTF8String];
}

@implementation MTRDeviceControllerDataStore {
    id<MTRDeviceControllerStorageDelegate> _storageDelegate;
    dispatch_queue_t _storageDelegateQueue;
    MTRDeviceController * _controller;
    // Array of nodes with resumption info, oldest-stored first.
    NSMutableArray<NSNumber *> * _nodesWithResumptionInfo;
}

- (instancetype)initWithController:(MTRDeviceController *)controller
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
                                              sharingType:MTRStorageSharingTypeSameIdentityAllowed];
    });
    if (resumptionNodeList != nil) {
        if (![resumptionNodeList isKindOfClass:[NSMutableArray class]]) {
            MTR_LOG_ERROR("List of CASE resumption node IDs is not an array");
            return nil;
        }
        _nodesWithResumptionInfo = resumptionNodeList;
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
                             sharingType:MTRStorageSharingTypeSameIdentityAllowed];
            [_nodesWithResumptionInfo removeObject:resumptionInfo.nodeID];
        }

        [_storageDelegate controller:_controller
                          storeValue:resumptionInfo
                              forKey:ResumptionByNodeIDKey(resumptionInfo.nodeID)
                       securityLevel:MTRStorageSecurityLevelSecure
                         sharingType:MTRStorageSharingTypeSameIdentityAllowed];
        [_storageDelegate controller:_controller
                          storeValue:resumptionInfo
                              forKey:ResumptionByResumptionIDKey(resumptionInfo.resumptionID)
                       securityLevel:MTRStorageSecurityLevelSecure
                         sharingType:MTRStorageSharingTypeSameIdentityAllowed];

        // Update our resumption info node list.
        [_nodesWithResumptionInfo addObject:resumptionInfo.nodeID];
        [_storageDelegate controller:_controller
                          storeValue:_nodesWithResumptionInfo
                              forKey:sResumptionNodeListKey
                       securityLevel:MTRStorageSecurityLevelSecure
                         sharingType:MTRStorageSharingTypeSameIdentityAllowed];
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
                                 sharingType:MTRStorageSharingTypeSameIdentityAllowed];
                [_storageDelegate controller:_controller
                           removeValueForKey:ResumptionByNodeIDKey(oldInfo.nodeID)
                               securityLevel:MTRStorageSecurityLevelSecure
                                 sharingType:MTRStorageSharingTypeSameIdentityAllowed];
            });
        }
    }

    [_nodesWithResumptionInfo removeAllObjects];
}

- (CHIP_ERROR)storeLastUsedNOC:(MTRCertificateTLVBytes)noc
{
    __block BOOL ok;
    dispatch_sync(_storageDelegateQueue, ^{
        ok = [_storageDelegate controller:_controller
                               storeValue:noc
                                   forKey:sLastUsedNOCKey
                            securityLevel:MTRStorageSecurityLevelSecure
                              sharingType:MTRStorageSharingTypeSameIdentityRequired];
    });
    return ok ? CHIP_NO_ERROR : CHIP_ERROR_PERSISTED_STORAGE_FAILED;
}

- (MTRCertificateTLVBytes _Nullable)fetchLastUsedNOC
{
    __block id data;
    dispatch_sync(_storageDelegateQueue, ^{
        data = [_storageDelegate controller:_controller
                                valueForKey:sLastUsedNOCKey
                              securityLevel:MTRStorageSecurityLevelSecure
                                sharingType:MTRStorageSharingTypeSameIdentityRequired];
    });

    if (data == nil) {
        return nil;
    }

    if (![data isKindOfClass:[NSData class]]) {
        return nil;
    }

    return data;
}

- (nullable MTRCASESessionResumptionInfo *)_findResumptionInfoWithKey:(NSString *)key
{
    __block id resumptionInfo;
    dispatch_sync(_storageDelegateQueue, ^{
        resumptionInfo = [_storageDelegate controller:_controller
                                          valueForKey:key
                                        securityLevel:MTRStorageSecurityLevelSecure
                                          sharingType:MTRStorageSharingTypeSameIdentityAllowed];
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

- (instancetype _Nullable)initWithCoder:(NSCoder *)decoder
{
    _nodeID = [decoder decodeObjectOfClass:[NSNumber class] forKey:sNodeIDKey];
    _resumptionID = [decoder decodeObjectOfClass:[NSData class] forKey:sResumptionIDKey];
    _sharedSecret = [decoder decodeObjectOfClass:[NSData class] forKey:sSharedSecretKey];
    _caseAuthenticatedTags = [decoder decodeObjectOfClass:[NSSet class] forKey:sCATsKey];
    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:self.nodeID forKey:sNodeIDKey];
    [coder encodeObject:self.resumptionID forKey:sResumptionIDKey];
    [coder encodeObject:self.sharedSecret forKey:sSharedSecretKey];
    [coder encodeObject:self.caseAuthenticatedTags forKey:sCATsKey];
}

@end

NSSet<Class> * MTRDeviceControllerStorageClasses()
{
    static NSSet * const sStorageClasses = [NSSet setWithArray:@[
        [NSNumber class], [NSData class], [NSSet class], [MTRCASESessionResumptionInfo class], [NSMutableArray class]
    ]];
    return sStorageClasses;
}
