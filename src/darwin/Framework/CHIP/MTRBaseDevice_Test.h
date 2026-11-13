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

#import <Foundation/Foundation.h>
#import <Matter/Matter.h>

#import "MTRDefines_Internal.h"

NS_ASSUME_NONNULL_BEGIN

#ifdef DEBUG
// Imported by MTRBaseDevice.mm, so the compiler checks these against the implementation.
@interface MTRBaseDevice ()
// Test function for whitebox testing
+ (id)CHIPEncodeAndDecodeNSObject:(id)object;

+ (void)unitTestResetInvokeRequestMessageCount;
+ (NSUInteger)unitTestInvokeRequestMessageCount;

// Forces the MaxPathsPerInvoke reported for batched invokes; 0 restores reading it from the session.
+ (void)unitTestSetMaxPathsPerInvokeOverride:(uint16_t)maxPathsPerInvoke;
@end
#endif

NS_ASSUME_NONNULL_END
