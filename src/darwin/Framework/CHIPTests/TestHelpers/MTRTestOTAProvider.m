/**
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRTestOTAProvider.h"

@implementation MTRTestOTAProvider
- (void)handleQueryImageForNodeID:(NSNumber *)nodeID
                       controller:(MTRDeviceController *)controller
                           params:(MTROTASoftwareUpdateProviderClusterQueryImageParams *)params
                       completion:(void (^)(MTROTASoftwareUpdateProviderClusterQueryImageResponseParams * _Nullable data,
                                      NSError * _Nullable error))completion
{
}

- (void)handleApplyUpdateRequestForNodeID:(NSNumber *)nodeID
                               controller:(MTRDeviceController *)controller
                                   params:(MTROTASoftwareUpdateProviderClusterApplyUpdateRequestParams *)params
                               completion:(void (^)(MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams * _Nullable data,
                                              NSError * _Nullable error))completion
{
}

- (void)handleNotifyUpdateAppliedForNodeID:(NSNumber *)nodeID
                                controller:(MTRDeviceController *)controller
                                    params:(MTROTASoftwareUpdateProviderClusterNotifyUpdateAppliedParams *)params
                                completion:(MTRStatusCompletion)completion
{
}

- (void)handleBDXTransferSessionBeginForNodeID:(NSNumber * _Nonnull)nodeID
                                    controller:(MTRDeviceController * _Nonnull)controller
                                fileDesignator:(NSString * _Nonnull)fileDesignator
                                        offset:(NSNumber * _Nonnull)offset
                                    completion:(MTRStatusCompletion)completion
{
}

- (void)handleBDXTransferSessionEndForNodeID:(NSNumber * _Nonnull)nodeID
                                  controller:(MTRDeviceController * _Nonnull)controller
                                       error:(NSError * _Nullable)error
{
}

- (void)handleBDXQueryForNodeID:(NSNumber * _Nonnull)nodeID
                     controller:(MTRDeviceController * _Nonnull)controller
                      blockSize:(NSNumber * _Nonnull)blockSize
                     blockIndex:(NSNumber * _Nonnull)blockIndex
                    bytesToSkip:(NSNumber * _Nonnull)bytesToSkip
                     completion:(void (^)(NSData * _Nullable data, BOOL isEOF))completion
{
}

@end
