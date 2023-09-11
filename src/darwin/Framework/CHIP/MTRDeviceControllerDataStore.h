/**
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>
#import <Matter/MTRDefines.h>
#import <Matter/MTRDeviceController.h>
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
MTR_HIDDEN
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
MTR_HIDDEN
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

@end

NS_ASSUME_NONNULL_END
