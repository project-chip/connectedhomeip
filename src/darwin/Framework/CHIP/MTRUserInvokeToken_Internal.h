/**
 *    Copyright (c) 2026 Project CHIP Authors
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

#import <Matter/MTRUserInvokeToken.h>

#import "MTRAsyncWorkQueue.h"

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger, MTRUserInvokeTokenUse) {
    /// The token is now tied to this invoke.
    MTRUserInvokeTokenUseTied,
    /// The token was already tied to another invoke; it cannot cancel this one.
    MTRUserInvokeTokenUseAlreadyUsed,
    /// The token was cancelled before it was used: its cancel returned YES, so this invoke must not be sent.
    MTRUserInvokeTokenUseCancelledBeforeUse,
};

@interface MTRUserInvokeToken ()

/// Ties the token to the queued work item of its invoke, so its cancel can cancel that item while it is unsent.
/// Ties nothing unless it returns MTRUserInvokeTokenUseTied.
- (MTRUserInvokeTokenUse)attachToWorkItem:(MTRAsyncWorkItem *)workItem queue:(MTRAsyncWorkQueue *)queue;

/// Ties the token to an invoke that has no work item to cancel, so from now on its cancel returns NO.  Ties nothing
/// unless it returns MTRUserInvokeTokenUseTied.
- (MTRUserInvokeTokenUse)markUsedWithoutWorkItem;

/// YES once -cancel was called before the token was tied to an invoke.
@property (nonatomic, readonly) BOOL cancelledBeforeUse;

@end

NS_ASSUME_NONNULL_END
