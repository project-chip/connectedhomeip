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

#import "MTRError.h"
#import "MTRError_Internal.h"

#import <app/MessageDef/StatusIB.h>
#import <app/util/af-enums.h>
#import <app/util/error-mapping.h>
#import <inet/InetError.h>
#import <lib/support/TypeTraits.h>

NSString * const MTRErrorDomain = @"MTRErrorDomain";

NSString * const MTRInteractionErrorDomain = @"MTRInteractionErrorDomain";

// Class for holding on to a CHIP_ERROR that we can use as the value
// in a dictionary.
@interface MTRErrorHolder : NSObject
@property (nonatomic, readonly) CHIP_ERROR error;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

- (instancetype)initWithError:(CHIP_ERROR)error;
@end

@implementation MTRError

+ (NSError *)errorForCHIPErrorCode:(CHIP_ERROR)errorCode
{
    if (errorCode == CHIP_NO_ERROR) {
        return nil;
    }

    if (errorCode.IsIMStatus()) {
        chip::app::StatusIB status(errorCode);
        return [MTRError errorForIMStatus:status];
    }

    NSMutableDictionary * userInfo = [[NSMutableDictionary alloc] init];
    MTRErrorCode code = MTRErrorCodeGeneralError;

    if (errorCode == CHIP_ERROR_INVALID_STRING_LENGTH) {
        code = MTRErrorCodeInvalidStringLength;
        [userInfo addEntriesFromDictionary:@{ NSLocalizedDescriptionKey : NSLocalizedString(@"A list length is invalid.", nil) }];
    } else if (errorCode == CHIP_ERROR_INVALID_INTEGER_VALUE) {
        code = MTRErrorCodeInvalidIntegerValue;
        [userInfo addEntriesFromDictionary:@{ NSLocalizedDescriptionKey : NSLocalizedString(@"Unexpected integer value.", nil) }];
    } else if (errorCode == CHIP_ERROR_INVALID_ARGUMENT) {
        code = MTRErrorCodeInvalidArgument;
        [userInfo addEntriesFromDictionary:@{ NSLocalizedDescriptionKey : NSLocalizedString(@"An argument is invalid.", nil) }];
    } else if (errorCode == CHIP_ERROR_INVALID_MESSAGE_LENGTH) {
        code = MTRErrorCodeInvalidMessageLength;
        [userInfo
            addEntriesFromDictionary:@{ NSLocalizedDescriptionKey : NSLocalizedString(@"A message length is invalid.", nil) }];
    } else if (errorCode == CHIP_ERROR_INCORRECT_STATE) {
        code = MTRErrorCodeInvalidState;
        [userInfo addEntriesFromDictionary:@{ NSLocalizedDescriptionKey : NSLocalizedString(@"Invalid object state.", nil) }];
    } else if (errorCode == CHIP_ERROR_INTEGRITY_CHECK_FAILED) {
        code = MTRErrorCodeIntegrityCheckFailed;
        [userInfo addEntriesFromDictionary:@{ NSLocalizedDescriptionKey : NSLocalizedString(@"Integrity check failed.", nil) }];
    } else if (errorCode == CHIP_ERROR_TIMEOUT) {
        code = MTRErrorCodeTimeout;
        [userInfo addEntriesFromDictionary:@{ NSLocalizedDescriptionKey : NSLocalizedString(@"Transaction timed out.", nil) }];
    } else if (errorCode == CHIP_ERROR_BUFFER_TOO_SMALL) {
        code = MTRErrorCodeBufferTooSmall;
        [userInfo addEntriesFromDictionary:@{ NSLocalizedDescriptionKey : NSLocalizedString(@"A buffer is too small.", nil) }];
    } else if (errorCode == CHIP_ERROR_FABRIC_EXISTS) {
        code = MTRErrorCodeFabricExists;
        [userInfo addEntriesFromDictionary:@{
            NSLocalizedDescriptionKey : NSLocalizedString(@"The device is already a member of this fabric.", nil)
        }];
    } else {
        code = MTRErrorCodeGeneralError;
        [userInfo addEntriesFromDictionary:@{
            NSLocalizedDescriptionKey :
                [NSString stringWithFormat:NSLocalizedString(@"Undefined error:%u.", nil), errorCode.AsInteger()],
            @"errorCode" : @(errorCode.AsInteger()),
        }];
    }

    userInfo[@"underlyingError"] = [[MTRErrorHolder alloc] initWithError:errorCode];

    return [NSError errorWithDomain:MTRErrorDomain code:code userInfo:userInfo];
    ;
}

+ (NSError *)errorForIMStatus:(const chip::app::StatusIB &)status
{
    if (status.IsSuccess()) {
        return nil;
    }

    NSString * description;
    using chip::Protocols::InteractionModel::Status;
    switch (status.mStatus) {
    case Status::Failure:
    default: {
        description = NSLocalizedString(@"Operation was not successful.", nil);
        break;
    }
    case Status::InvalidSubscription: {
        description = NSLocalizedString(@"Subscription ID is not active.", nil);
        break;
    }
    case Status::UnsupportedAccess: {
        description = NSLocalizedString(@"The sender of the action or command does not have authorization or access.", nil);
        break;
    }
    case Status::UnsupportedEndpoint: {
        description = NSLocalizedString(@"The endpoint indicated is unsupported on the node.", nil);
        break;
    }
    case Status::InvalidAction: {
        description = NSLocalizedString(
            @"The action is malformed, has missing fields, or fields with invalid values. Action not carried out.", nil);
        break;
    }
    case Status::UnsupportedCommand: {
        description = NSLocalizedString(
            @"The specified action or command indicated is not supported on the device. Command or action not carried out.", nil);
        break;
    }
    case Status::InvalidCommand: {
        description = NSLocalizedString(
            @"The cluster command is malformed, has missing fields, or fields with invalid values. Command not carried out.", nil);
        break;
    }
    case Status::UnsupportedAttribute: {
        description
            = NSLocalizedString(@"The specified attribute or attribute data field or entry does not exist on the device.", nil);
        break;
    }
    case Status::ConstraintError: {
        description = NSLocalizedString(@"Out of range error or set to a reserved value.", nil);
        break;
    }
    case Status::UnsupportedWrite: {
        description = NSLocalizedString(@"Attempt to write a read-only attribute.", nil);
        break;
    }
    case Status::ResourceExhausted: {
        description = NSLocalizedString(@"An action or operation failed due to insufficient available resources. ", nil);
        break;
    }
    case Status::NotFound: {
        description = NSLocalizedString(@"The indicated data field or entry could not be found.", nil);
        break;
    }
    case Status::UnreportableAttribute: {
        description = NSLocalizedString(@"Reports cannot be issued for this attribute.", nil);
        break;
    }
    case Status::InvalidDataType: {
        description = NSLocalizedString(
            @"The data type indicated is undefined or invalid for the indicated data field. Command or action not carried out.",
            nil);
        break;
    }
    case Status::UnsupportedRead: {
        description = NSLocalizedString(@"Attempt to read a write-only attribute.", nil);
        break;
    }
    case Status::DataVersionMismatch: {
        description = NSLocalizedString(@"Cluster instance data version did not match request path.", nil);
        break;
    }
    case Status::Timeout: {
        description = NSLocalizedString(@"The transaction was aborted due to time being exceeded.", nil);
        break;
    }
    case Status::Busy: {
        description = NSLocalizedString(
            @"The receiver is busy processing another action that prevents the execution of the incoming action.", nil);
        break;
    }
    case Status::UnsupportedCluster: {
        description = NSLocalizedString(@"The cluster indicated is not supported", nil);
        break;
    }
    // Gap in values is intentional.
    case Status::NoUpstreamSubscription: {
        description = NSLocalizedString(@"Proxy does not have a subscription to the source.", nil);
        break;
    }
    case Status::NeedsTimedInteraction: {
        description = NSLocalizedString(@"An Untimed Write or Untimed Invoke interaction was used for an attribute or command that "
                                        @"requires a Timed Write or Timed Invoke.",
            nil);
        break;
    }
    case Status::UnsupportedEvent: {
        description = NSLocalizedString(@"The event indicated is unsupported on the cluster.", nil);
        break;
    }
    }

    NSMutableDictionary * userInfo = [[NSMutableDictionary alloc] init];
    userInfo[NSLocalizedDescriptionKey] = description;
    if (status.mClusterStatus.HasValue()) {
        userInfo[@"clusterStatus"] = @(status.mClusterStatus.Value());
    }

    return [NSError errorWithDomain:MTRInteractionErrorDomain code:chip::to_underlying(status.mStatus) userInfo:userInfo];
}

+ (CHIP_ERROR)errorToCHIPErrorCode:(NSError * _Nullable)error
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
        if (underlyingError != nil && [underlyingError isKindOfClass:[MTRErrorHolder class]]) {
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
    case MTRErrorCodeBufferTooSmall:
        code = CHIP_ERROR_BUFFER_TOO_SMALL.AsInteger();
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

@end

@implementation MTRErrorHolder

- (instancetype)initWithError:(CHIP_ERROR)error
{
    if (!(self = [super init])) {
        return nil;
    }

    _error = error;
    return self;
}

@end
