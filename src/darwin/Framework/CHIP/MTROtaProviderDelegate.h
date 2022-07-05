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
 * The protocol definition for the MTROtaProviderDelegate
 *
 * All delegate methods will be called on the supplied Delegate Queue.
 */
@protocol MTROtaProviderDelegate <NSObject>
@optional
/**
 * Notify the delegate when query image command is received
 *
 */
// cyang: Determine what the params should be to pass all necessary data over
- (void)handleQueryImage:(CHIPOtaSoftwareUpdateProviderClusterQueryImageParams *)command
       completionHandler:
           (void (^)(NSError * _Nullable error,
               CHIPOtaSoftwareUpdateProviderClusterQueryImageResponseParams * _Nullable responseParams))completionHandler;

/**
 * Notify the delegate when apply update request command is received
 *
 */
- (void)handleApplyUpdateRequest:(CHIPOtaSoftwareUpdateProviderClusterApplyUpdateRequestParams *)command
               completionHandler:
                   (void (^)(NSError * _Nullable error,
                       CHIPOtaSoftwareUpdateProviderClusterApplyUpdateResponseParams * _Nullable responseParams))completionHandler;

/**
 * Notify the delegate when notify update applied command is received
 *
 */
- (void)handleNotifyUpdateApplied:(CHIPOtaSoftwareUpdateProviderClusterNotifyUpdateAppliedParams *)command
                completionHandler:(void (^)(NSError * _Nullable error))completionHandler;

@end

NS_ASSUME_NONNULL_END
