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

#import "MTRCommandPayloadsObjc_Private.h"
#import "MTRBackwardsCompatShims.h"
#import "MTRBaseDevice_Internal.h"
#import "MTRCommandPayloadExtensions_Internal.h"
#import "MTRCommandPayloadsObjc.h"
#import "MTRCommandPayloads_Private.h"
#import "MTRDefines_Internal.h"
#import "MTRError_Internal.h"
#import "MTRLogging_Internal.h"
#import "NSDataSpanConversion.h"
#import "NSStringSpanConversion.h"

#include <app/data-model/Decode.h>
#include <app/data-model/ListLargeSystemExtensions.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <system/TLVPacketBufferBackingStore.h>

NS_ASSUME_NONNULL_BEGIN
// Nothing here for now, but leaving this file in place in case we need to add
// something.
static void LogAndConvertDecodingError(CHIP_ERROR err, NSError * __autoreleasing * error) __attribute__((unused));
static void LogAndConvertDecodingError(CHIP_ERROR err, NSError * __autoreleasing * error)
{
    NSString * errorStr = [NSString stringWithFormat:@"Command payload decoding failed: %s", err.AsString()];
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (error != nil) {
        NSDictionary * userInfo = @ { NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:userInfo];
    }
}

NS_ASSUME_NONNULL_END
