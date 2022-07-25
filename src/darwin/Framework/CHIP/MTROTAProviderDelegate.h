/**
 *
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

#import <Foundation/Foundation.h>
#import <Matter/Matter.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * The protocol definition for the MTROTAProviderDelegate
 *
 * All delegate methods will be called on the supplied Delegate Queue.
 */
@protocol MTROTAProviderDelegate <NSObject>
@required
/**
 * Notify the delegate when query image command is received
 *
 */
- (void)handleQueryImage:(MTROtaSoftwareUpdateProviderClusterQueryImageParams *)params
       completionHandler:(void (^)(MTROtaSoftwareUpdateProviderClusterQueryImageResponseParams * _Nullable data,
                             NSError * _Nullable error))completionHandler;

/**
 * Notify the delegate when apply update request command is received
 *
 */
- (void)handleApplyUpdateRequest:(MTROtaSoftwareUpdateProviderClusterApplyUpdateRequestParams *)params
               completionHandler:(void (^)(MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams * _Nullable data,
                                     NSError * _Nullable error))completionHandler;

/**
 * Notify the delegate when notify update applied command is received
 *
 */
- (void)handleNotifyUpdateApplied:(MTROtaSoftwareUpdateProviderClusterNotifyUpdateAppliedParams *)params
                completionHandler:(StatusCompletion)completionHandler;

/**
 * Notify the delegate when a BDX Session starts
 *
 */
- (void)handleBDXTransferSessionBegin:(NSString * _Nonnull)fileDesignator
                               offset:(NSNumber * _Nonnull)offset
                    completionHandler:(void (^)(NSError * error))completionHandler;

/**
 * Notify the delegate when a BDX Session ends
 *
 */
- (void)handleBDXTransferSessionEnd:(NSError * _Nullable)error;

/**
 * Notify the delegate when a BDX Query message has been received
 *
 */
- (void)handleBDXQuery:(NSNumber * _Nonnull)blockSize
            blockIndex:(NSNumber * _Nonnull)blockIndex
           bytesToSkip:(NSNumber * _Nonnull)bytesToSkip
     completionHandler:(void (^)(NSData * _Nullable data, BOOL isEOF))completionHandler;
@end

NS_ASSUME_NONNULL_END
