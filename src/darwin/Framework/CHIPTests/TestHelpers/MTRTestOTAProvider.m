/**
 *    Copyright (c) 2022 Project CHIP Authors
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
