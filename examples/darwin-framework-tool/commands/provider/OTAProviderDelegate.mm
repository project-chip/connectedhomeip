/*
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

#include "OTAProviderDelegate.h"
#import <Matter/Matter.h>


@interface OTAProviderDelegate ()
@end

@implementation OTAProviderDelegate
- (void)handleQueryImage:(MTROtaSoftwareUpdateProviderClusterQueryImageParams * _Nonnull)params
       completionHandler:(void (^ _Nonnull)(MTROtaSoftwareUpdateProviderClusterQueryImageResponseParams * _Nullable data,
                             NSError * _Nullable error))completionHandler
{
    NSLog(@"handleQueryImage: %@", params);

}

- (void)handleApplyUpdateRequest:(MTROtaSoftwareUpdateProviderClusterApplyUpdateRequestParams * _Nonnull)params
               completionHandler:(void (^ _Nonnull)(MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams * _Nullable data,
                                     NSError * _Nullable error))completionHandler
{
    NSLog(@"handleApplyUpdateRequest: %@", params);
}

- (void)handleNotifyUpdateApplied:(MTROtaSoftwareUpdateProviderClusterNotifyUpdateAppliedParams * _Nonnull)params
                completionHandler:(StatusCompletion _Nonnull)completionHandler
{
    NSLog(@"handleNotifyUpdateApplied: %@", params);
}

@end
