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

#import "CHIPError.h"
#import "CHIPError_Internal.h"

#import <app/util/af-enums.h>
#import <inet/InetError.h>

NSString * const CHIPErrorDomain = @"CHIPErrorDomain";

@implementation CHIPError

+ (NSError *)errorForCHIPErrorCode:(CHIP_ERROR)errorCode
{
    if (errorCode == CHIP_ERROR_INVALID_STRING_LENGTH) {
        return [NSError errorWithDomain:CHIPErrorDomain
                                   code:CHIPErrorCodeInvalidStringLength
                               userInfo:@{ NSLocalizedDescriptionKey : NSLocalizedString(@"A list length is invalid.", nil) }];
    }

    if (errorCode == CHIP_ERROR_INVALID_INTEGER_VALUE) {
        return [NSError errorWithDomain:CHIPErrorDomain
                                   code:CHIPErrorCodeInvalidIntegerValue
                               userInfo:@{ NSLocalizedDescriptionKey : NSLocalizedString(@"Unexpected integer value.", nil) }];
    }

    if (errorCode == CHIP_ERROR_INVALID_ARGUMENT) {
        return [NSError errorWithDomain:CHIPErrorDomain
                                   code:CHIPErrorCodeInvalidArgument
                               userInfo:@{ NSLocalizedDescriptionKey : NSLocalizedString(@"An argument is invalid.", nil) }];
    }

    if (errorCode == CHIP_ERROR_INVALID_MESSAGE_LENGTH) {
        return [NSError errorWithDomain:CHIPErrorDomain
                                   code:CHIPErrorCodeInvalidMessageLength
                               userInfo:@{ NSLocalizedDescriptionKey : NSLocalizedString(@"A message length is invalid.", nil) }];
    }

    if (errorCode == CHIP_ERROR_INCORRECT_STATE) {
        return [NSError errorWithDomain:CHIPErrorDomain
                                   code:CHIPErrorCodeInvalidState
                               userInfo:@{ NSLocalizedDescriptionKey : NSLocalizedString(@"Invalid object state.", nil) }];
    }

    if (errorCode == CHIP_ERROR_INTEGRITY_CHECK_FAILED) {
        return [NSError errorWithDomain:CHIPErrorDomain
                                   code:CHIPErrorCodeIntegrityCheckFailed
                               userInfo:@{ NSLocalizedDescriptionKey : NSLocalizedString(@"Integrity check failed.", nil) }];
    }

    if (errorCode == CHIP_NO_ERROR) {
        return [NSError errorWithDomain:CHIPErrorDomain
                                   code:CHIPSuccess
                               userInfo:@{ NSLocalizedDescriptionKey : NSLocalizedString(@"Success.", nil) }];
    }

    return [NSError errorWithDomain:CHIPErrorDomain
                               code:CHIPErrorCodeUndefinedError
                           userInfo:@{
                               NSLocalizedDescriptionKey :
                                   [NSString stringWithFormat:NSLocalizedString(@"Undefined error:%u.", nil), errorCode.AsInteger()]
                           }];
    ;
}

+ (NSError *)errorForZCLErrorCode:(uint8_t)errorCode
{
    switch (errorCode) {
    case EMBER_ZCL_STATUS_DUPLICATE_EXISTS:
        return [NSError
            errorWithDomain:CHIPErrorDomain
                       code:CHIPErrorCodeDuplicateExists
                   userInfo:@{ NSLocalizedDescriptionKey : NSLocalizedString(@"A Duplicate entry or setting exists.", nil) }];
    case EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT:
        return
            [NSError errorWithDomain:CHIPErrorDomain
                                code:CHIPErrorCodeUnsupportedEndpoint
                            userInfo:@{ NSLocalizedDescriptionKey : NSLocalizedString(@"Target endpoint does not exist.", nil) }];
    case EMBER_ZCL_STATUS_UNSUPPORTED_COMMAND:
        return
            [NSError errorWithDomain:CHIPErrorDomain
                                code:CHIPErrorCodeUnsupportedCommand
                            userInfo:@{
                                NSLocalizedDescriptionKey : NSLocalizedString(@"Command is not supported on target cluster.", nil)
                            }];
    case EMBER_ZCL_STATUS_INVALID_COMMAND:
        return [NSError errorWithDomain:CHIPErrorDomain
                                   code:CHIPErrorCodeInvalidCommand
                               userInfo:@{ NSLocalizedDescriptionKey : NSLocalizedString(@"Command payload is invalid.", nil) }];
    case EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE:
        return [NSError errorWithDomain:CHIPErrorDomain
                                   code:CHIPErrorCodeUnsupportedAttribute
                               userInfo:@{ NSLocalizedDescriptionKey : NSLocalizedString(@"Attribute is not supported.", nil) }];
    case EMBER_ZCL_STATUS_CONSTRAINT_ERROR:
        return [NSError errorWithDomain:CHIPErrorDomain
                                   code:CHIPErrorCodeConstraintError
                               userInfo:@{ NSLocalizedDescriptionKey : NSLocalizedString(@"Value out of range.", nil) }];
    case EMBER_ZCL_STATUS_UNSUPPORTED_WRITE:
        return [NSError
            errorWithDomain:CHIPErrorDomain
                       code:CHIPErrorCodeUnsupportedWrite
                   userInfo:@{ NSLocalizedDescriptionKey : NSLocalizedString(@"Attempt to write read-only attribute.", nil) }];
    case EMBER_ZCL_STATUS_UNSUPPORTED_CLUSTER:
        return
            [NSError errorWithDomain:CHIPErrorDomain
                                code:CHIPErrorCodeUnsupportedCluster
                            userInfo:@{
                                NSLocalizedDescriptionKey : NSLocalizedString(@"Cluster is not supported on target endpoint.", nil)
                            }];
    default:
        return [NSError errorWithDomain:CHIPErrorDomain
                                   code:CHIPErrorCodeUndefinedError
                               userInfo:@{
                                   NSLocalizedDescriptionKey : [NSString
                                       stringWithFormat:NSLocalizedString(@"Undefined data model error:%u.", nil), errorCode]
                               }];
    }
}

+ (CHIP_ERROR)errorToCHIPErrorCode:(NSError *)error
{
    if (error.domain != CHIPErrorDomain) {
        return CHIP_ERROR_INTERNAL;
    }

    switch (error.code) {
    case CHIPErrorCodeInvalidStringLength:
        return CHIP_ERROR_INVALID_STRING_LENGTH;
    case CHIPErrorCodeInvalidIntegerValue:
        return CHIP_ERROR_INVALID_INTEGER_VALUE;
    case CHIPErrorCodeInvalidArgument:
        return CHIP_ERROR_INVALID_ARGUMENT;
    case CHIPErrorCodeInvalidMessageLength:
        return CHIP_ERROR_INVALID_MESSAGE_LENGTH;
    case CHIPErrorCodeInvalidState:
        return CHIP_ERROR_INCORRECT_STATE;
    case CHIPErrorCodeIntegrityCheckFailed:
        return CHIP_ERROR_INTEGRITY_CHECK_FAILED;
    case CHIPSuccess:
        return CHIP_NO_ERROR;
    default:
        return CHIP_ERROR_INTERNAL;
    }
}

+ (uint8_t)errorToZCLErrorCode:(NSError * _Nullable)error
{
    // If this is changed, change CHIPErrorTestUtils' version of
    // errorToZCLErrorCode too.
    if (error == nil) {
        return EMBER_ZCL_STATUS_SUCCESS;
    }
    if (error.domain != CHIPErrorDomain) {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    switch (error.code) {
    case CHIPErrorCodeDuplicateExists:
        return EMBER_ZCL_STATUS_DUPLICATE_EXISTS;
    case CHIPErrorCodeUnsupportedEndpoint:
        return EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT;
    case CHIPErrorCodeUnsupportedCommand:
        return EMBER_ZCL_STATUS_UNSUPPORTED_COMMAND;
    case CHIPErrorCodeInvalidCommand:
        return EMBER_ZCL_STATUS_INVALID_COMMAND;
    case CHIPErrorCodeUnsupportedAttribute:
        return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
    case CHIPErrorCodeConstraintError:
        return EMBER_ZCL_STATUS_CONSTRAINT_ERROR;
    case CHIPErrorCodeUnsupportedWrite:
        return EMBER_ZCL_STATUS_UNSUPPORTED_WRITE;
    case CHIPErrorCodeUnsupportedCluster:
        return EMBER_ZCL_STATUS_UNSUPPORTED_CLUSTER;
    case CHIPSuccess:
        return EMBER_ZCL_STATUS_SUCCESS;
    default:
        return EMBER_ZCL_STATUS_FAILURE;
    }
}
@end
