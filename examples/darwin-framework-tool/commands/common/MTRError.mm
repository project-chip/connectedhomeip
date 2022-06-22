/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#import <Matter/Matter.h>
#import "MTRError_Internal.h"

#import <app/MessageDef/StatusIB.h>
#import <app/util/af-enums.h>
#import <app/util/error-mapping.h>
#import <inet/InetError.h>
#import <lib/support/TypeTraits.h>

// Stolen for now from the framework, need to export this properly.
@interface MTRErrorHolder : NSObject
@property (nonatomic, readonly) CHIP_ERROR error;
@end

CHIP_ERROR MTRErrorToCHIPErrorCode(NSError * error)
{
    if (error == nil) {
        return CHIP_NO_ERROR;
    }

    if (error.domain == MTRInteractionErrorDomain) {
        chip::app::StatusIB status(static_cast<chip::Protocols::InteractionModel::Status>(error.code));
        if (error.userInfo != nil && error.userInfo[@"clusterStatus"] != nil) {
            status.mClusterStatus.Emplace([error.userInfo[@"clusterStatus"] unsignedCharValue]);
        }
        return status.ToChipError();
    }

    if (error.domain != MTRErrorDomain) {
        return CHIP_ERROR_INTERNAL;
    }

    if (error.userInfo != nil) {
        id underlyingError = error.userInfo[@"underlyingError"];
        if (underlyingError != nil && [underlyingError isKindOfClass: [MTRErrorHolder class]]) {
            return ((MTRErrorHolder *) underlyingError).error;
        }
    }

    chip::ChipError::StorageType code;
    switch (error.code) {
    case MTRErrorCodeInvalidStringLength:
        code = CHIP_ERROR_INVALID_STRING_LENGTH.AsInteger();
        break;
    case MTRErrorCodeInvalidIntegerValue:
        code = CHIP_ERROR_INVALID_INTEGER_VALUE.AsInteger();
        break;
    case MTRErrorCodeInvalidArgument:
        code = CHIP_ERROR_INVALID_ARGUMENT.AsInteger();
        break;
    case MTRErrorCodeInvalidMessageLength:
        code = CHIP_ERROR_INVALID_MESSAGE_LENGTH.AsInteger();
        break;
    case MTRErrorCodeInvalidState:
        code = CHIP_ERROR_INCORRECT_STATE.AsInteger();
        break;
    case MTRErrorCodeIntegrityCheckFailed:
        code = CHIP_ERROR_INTEGRITY_CHECK_FAILED.AsInteger();
        break;
    case MTRErrorCodeTimeout:
        code = CHIP_ERROR_TIMEOUT.AsInteger();
        break;
    case MTRErrorCodeGeneralError: {
        if (error.userInfo != nil && error.userInfo[@"errorCode"] != nil) {
            code = static_cast<decltype(code)>([error.userInfo[@"errorCode"] unsignedLongValue]);
            break;
        }
        // Weird error we did not create.  Fall through.
    default:
        code = CHIP_ERROR_INTERNAL.AsInteger();
        break;
    }
    }

    return chip::ChipError(code);
}
