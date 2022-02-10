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

#import <app/MessageDef/StatusIB.h>
#import <app/util/af-enums.h>
#import <app/util/error-mapping.h>
#import <inet/InetError.h>
#import <lib/support/TypeTraits.h>

NSString * const CHIPErrorDomain = @"CHIPErrorDomain";

NSString * const MatterInteractionErrorDomain = @"MatterInteractionErrorDomain";

@implementation CHIPError

+ (NSError *)errorForCHIPErrorCode:(CHIP_ERROR)errorCode
{
    if (errorCode == CHIP_NO_ERROR) {
        return nil;
    }

    if (errorCode.IsIMStatus()) {
        chip::app::StatusIB status(errorCode);
        return [CHIPError errorForIMStatus:status];
    }

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

    return [NSError errorWithDomain:CHIPErrorDomain
                               code:CHIPErrorCodeGeneralError
                           userInfo:@{
                               NSLocalizedDescriptionKey : [NSString
                                   stringWithFormat:NSLocalizedString(@"Undefined error:%u.", nil), errorCode.AsInteger()],
                               @"errorCode" : @(errorCode.AsInteger()),
                           }];
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

    return [NSError errorWithDomain:MatterInteractionErrorDomain code:chip::to_underlying(status.mStatus) userInfo:userInfo];
}

+ (CHIP_ERROR)errorToCHIPErrorCode:(NSError * _Nullable)error
{
    if (error == nil) {
        return CHIP_NO_ERROR;
    }

    if (error.domain == MatterInteractionErrorDomain) {
        chip::app::StatusIB status(static_cast<chip::Protocols::InteractionModel::Status>(error.code));
        if (error.userInfo[@"clusterStatus"] != nil) {
            status.mClusterStatus.Emplace([error.userInfo[@"clusterStatus"] unsignedCharValue]);
        }
        return status.ToChipError();
    }

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
    default:
        return CHIP_ERROR_INTERNAL;
    }
}

@end
