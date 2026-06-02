/**
 *    Copyright (c) 2024 Project CHIP Authors
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
#import <Matter/MTRError.h>

#import "MTRDefines_Internal.h"

#include <string.h>

NS_ASSUME_NONNULL_BEGIN

// Basename-stripping helper used by NSError(Matter) when surfacing the underlying CHIP_ERROR
// source file. Defined here (rather than file-locally in MTRError.mm) so unit tests can
// exercise the path-stripping in isolation without going through CHIP_ERROR construction.
// Production callers in MTRError.mm invoke this same function -- there is one implementation,
// so test coverage of MTRErrorBasenameForPath does protect against regressions in the
// production lookup path.
//
// Pointer-comparison note: relational comparison of two pointers is only well-defined when
// both point into the same array. The two strrchr results below either point into path or are
// NULL, so the NULL cases are handled explicitly to avoid comparing NULL against a valid
// pointer (undefined behavior).
static inline NSString * _Nullable MTRErrorBasenameForPath(const char * _Nullable path)
{
    if (path == NULL || path[0] == '\0') {
        return nil;
    }
    const char * fwdSlash = strrchr(path, '/');
    const char * backSlash = strrchr(path, '\\');
    const char * basename;
    if (fwdSlash == NULL) {
        basename = backSlash;
    } else if (backSlash == NULL) {
        basename = fwdSlash;
    } else {
        basename = (fwdSlash > backSlash) ? fwdSlash : backSlash;
    }
    return @((basename != NULL) ? basename + 1 : path);
}

MTR_TESTABLE
@interface MTRError : NSObject

+ (NSError *)errorWithCode:(MTRErrorCode)code;

// For tests only, since we can't use CHIP_ERROR from there.  The "code"s used
// here are integer representations of CHIP_ERROR.  Otherwise these functions
// are just like errorForCHIPErrorCode and errorToCHIPErrorCode.
+ (NSError *)errorForCHIPIntegerCode:(uint32_t)code;
+ (uint32_t)errorToCHIPIntegerCode:(NSError * _Nullable)error;

// Test-only overload exposing additionalUserInfo plumbing without needing CHIP_ERROR. Mirrors
// MTRError_Internal.h's errorForCHIPErrorCode:logContext:additionalUserInfo: so tests can verify
// the merge path on every code class (general, IM-status, etc).
+ (NSError *)errorForCHIPIntegerCode:(uint32_t)code
                  additionalUserInfo:(nullable NSDictionary<NSErrorUserInfoKey, id> *)additionalUserInfo;

@end

NS_ASSUME_NONNULL_END
