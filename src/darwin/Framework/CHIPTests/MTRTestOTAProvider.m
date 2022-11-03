/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRTestOTAProvider.h"

@interface MTRTestOTAProvider ()
@end

@implementation MTRTestOTAProvider
- (void)handleQueryImageForNodeID:(NSNumber *)nodeID
                       controller:(MTRDeviceController *)controller
                           params:(MTROtaSoftwareUpdateProviderClusterQueryImageParams *)params
                completionHandler:(MTRQueryImageCompletionHandler)completionHandler
{
}

- (void)handleApplyUpdateRequestForNodeID:(NSNumber *)nodeID
                               controller:(MTRDeviceController *)controller
                                   params:(MTROtaSoftwareUpdateProviderClusterApplyUpdateRequestParams *)params
                        completionHandler:(MTRApplyUpdateRequestCompletionHandler)completionHandler
{
}

- (void)handleNotifyUpdateAppliedForNodeID:(NSNumber *)nodeID
                                controller:(MTRDeviceController *)controller
                                    params:(MTROtaSoftwareUpdateProviderClusterNotifyUpdateAppliedParams *)params
                         completionHandler:(StatusCompletion)completionHandler
{
}

- (void)handleBDXTransferSessionBeginForNodeID:(NSNumber * _Nonnull)nodeID
                                    controller:(MTRDeviceController * _Nonnull)controller
                                fileDesignator:(NSString * _Nonnull)fileDesignator
                                        offset:(NSNumber * _Nonnull)offset
                             completionHandler:(StatusCompletion)completionHandler
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
              completionHandler:(MTRBDXQueryCompletionHandler)completionHandler
{
}

@end
