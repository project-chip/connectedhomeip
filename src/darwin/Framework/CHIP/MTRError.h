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

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN
FOUNDATION_EXPORT NSErrorDomain const MTRErrorDomain;

FOUNDATION_EXPORT NSErrorDomain const MTRInteractionErrorDomain;

/**
 * MTRErrorDomain contains errors caused by data processing the framework
 * itself is performing.  These can be caused by invalid values provided to a
 * framework API, failure to decode an incoming message, and so forth.
 *
 * This error domain also contains errors that are communicated via success
 * responses from a server but mapped to an error on the client.
 *
 * Errors reported by the server side of a Matter interaction via the normal
 * Matter error-reporting mechanisms use MTRInteractionErrorDomain instead.
 */
// clang-format off
typedef NS_ERROR_ENUM(MTRErrorDomain, MTRErrorCode){
    /**
     * MTRErrorCodeGeneralError represents a generic Matter error with no
     * further categorization.
     *
     * The userInfo will have a key named @"errorCode" whose value will be an
     * integer representing the underlying Matter error code.  These integer
     * values should not be assumed to be stable across releases, but may be
     * useful in logging and debugging.
     */
    MTRErrorCodeGeneralError         = 1,
    MTRErrorCodeInvalidStringLength  = 2,
    MTRErrorCodeInvalidIntegerValue  = 3,
    MTRErrorCodeInvalidArgument      = 4,
    MTRErrorCodeInvalidMessageLength = 5,
    MTRErrorCodeInvalidState         = 6,
    MTRErrorCodeWrongAddressType     = 7,
    MTRErrorCodeIntegrityCheckFailed = 8,
    MTRErrorCodeTimeout              = 9,
    MTRErrorCodeBufferTooSmall       = 10,
    /**
     * MTRErrorCodeFabricExists is returned when trying to commission a device
     * into a fabric when it's already part of that fabric.
     */
    MTRErrorCodeFabricExists         = 11,
};
// clang-format on

/**
 * MTRInteractionErrorDomain contains errors that represent a Matter
 * StatusIB error.  These represent errors reported by the other side of a
 * Matter interaction.
 *
 * When the code is MTRInteractionErrorCodeFailure the userInfo may have a
 * key named @"clusterStatus" whose value is the cluster-specific status that
 * was reported.  This key will be absent if there was no cluster-specific
 * status.
 */
// clang-format off
typedef NS_ERROR_ENUM(MTRInteractionErrorDomain, MTRInteractionErrorCode){
    // These values come from the general status code table in the Matter
    // Interaction Model specification.
    MTRInteractionErrorCodeFailure                = 0x01,
    MTRInteractionErrorCodeInvalidSubscription    = 0x7d,
    MTRInteractionErrorCodeUnsupportedAccess      = 0x7e,
    MTRInteractionErrorCodeUnsupportedEndpoint    = 0x7f,
    MTRInteractionErrorCodeInvalidAction          = 0x80,
    MTRInteractionErrorCodeUnsupportedCommand     = 0x81,
    MTRInteractionErrorCodeInvalidCommand         = 0x85,
    MTRInteractionErrorCodeUnsupportedAttribute   = 0x86,
    MTRInteractionErrorCodeConstraintError        = 0x87,
    MTRInteractionErrorCodeUnsupportedWrite       = 0x88,
    MTRInteractionErrorCodeResourceExhausted      = 0x89,
    MTRInteractionErrorCodeNotFound               = 0x8b,
    MTRInteractionErrorCodeUnreportableAttribute  = 0x8c,
    MTRInteractionErrorCodeInvalidDataType        = 0x8d,
    MTRInteractionErrorCodeUnsupportedRead        = 0x8f,
    MTRInteractionErrorCodeDataVersionMismatch    = 0x92,
    MTRInteractionErrorCodeTimeout                = 0x94,
    MTRInteractionErrorCodeBusy                   = 0x9c,
    MTRInteractionErrorCodeUnsupportedCluster     = 0xc3,
    MTRInteractionErrorCodeNoUpstreamSubscription = 0xc5,
    MTRInteractionErrorCodeNeedsTimedInteraction  = 0xc6,
    MTRInteractionErrorCodeUnsupportedEvent       = 0xc7,
    MTRInteractionErrorCodePathsExhausted         = 0xc8,
    MTRInteractionErrorCodeTimedRequestMismatch   = 0xc9,
    MTRInteractionErrorCodeFailsafeRequired       = 0xca,
};
// clang-format on

NS_ASSUME_NONNULL_END
