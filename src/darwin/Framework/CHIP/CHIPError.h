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
FOUNDATION_EXPORT NSErrorDomain const CHIPErrorDomain;

FOUNDATION_EXPORT NSErrorDomain const MatterInteractionErrorDomain;

/**
 * ChipErrorDomain contains errors caused by data processing the framework
 * itself is performing.  These can be caused by invalid values provided to a
 * framework API, failure to decode an incoming message, and so forth.
 *
 * Errors reported by the other side of a Matter interaction use
 * MatterInteractionErrorDomain instead.
 */
// clang-format off
typedef NS_ERROR_ENUM(CHIPErrorDomain, CHIPErrorCode){
    /**
     * CHIPErrorCodeGeneralError represents a generic Matter error with no
     * further categorization.
     *
     * The userInfo will have a key named @"errorCode" whose value will be an
     * integer representing the underlying Matter error code.  These integer
     * values should not be assumed to be stable across releases, but may be
     * useful in logging and debugging.
     */
    CHIPErrorCodeGeneralError         = 1,
    CHIPErrorCodeInvalidStringLength  = 2,
    CHIPErrorCodeInvalidIntegerValue  = 3,
    CHIPErrorCodeInvalidArgument      = 4,
    CHIPErrorCodeInvalidMessageLength = 5,
    CHIPErrorCodeInvalidState         = 6,
    CHIPErrorCodeWrongAddressType     = 7,
    CHIPErrorCodeIntegrityCheckFailed = 8,
    CHIPErrorCodeTimeout              = 9,
};
// clang-format on

/**
 * MatterInteractionErrorDomain contains errors that represent a Matter
 * StatusIB error.  These represent errors reported by the other side of a
 * Matter interaction.
 *
 * When the code is MatterInteractionErrorCodeFailure the userInfo may have a
 * key named @"clusterStatus" whose value is the cluster-specific status that
 * was reported.  This key will be absent if there was no cluster-specific
 * status.
 */
// clang-format off
typedef NS_ERROR_ENUM(MatterInteractionErrorDomain, MatterInteractionErrorCode){
    // These values come from the general status code table in the Matter
    // Interaction Model specification.  Do not change these values unless the
    // specification changes.
    MatterInteractionErrorCodeFailure                = 0x01,
    MatterInteractionErrorCodeInvalidSubscription    = 0x7d,
    MatterInteractionErrorCodeUnsupportedAccess      = 0x7e,
    MatterInteractionErrorCodeUnsupportedEndpoint    = 0x7f,
    MatterInteractionErrorCodeInvalidAction          = 0x80,
    MatterInteractionErrorCodeUnsupportedCommand     = 0x81,
    // Gap in values is intentional.
    MatterInteractionErrorCodeInvalidCommand         = 0x85,
    MatterInteractionErrorCodeUnsupportedAttribute   = 0x86,
    MatterInteractionErrorCodeConstraintError        = 0x87,
    MatterInteractionErrorCodeUnsupportedWrite       = 0x88,
    MatterInteractionErrorCodeResourceExhausted      = 0x89,
    // Gap in values is intentional.
    MatterInteractionErrorCodeNotFound               = 0x8b,
    MatterInteractionErrorCodeUnreportableAttribute  = 0x8c,
    MatterInteractionErrorCodeInvalidDataType        = 0x8d,
    // Gap in values is intentional.
    MatterInteractionErrorCodeUnsupportedRead        = 0x8f,
    // Gap in values is intentional.
    MatterInteractionErrorCodeDataVersionMismatch    = 0x92,
    // Gap in values is intentional.
    MatterInteractionErrorCodeTimeout                = 0x94,
    // Gap in values is intentional.
    MatterInteractionErrorCodeBusy                   = 0x9c,
    // Gap in values is intentional.
    MatterInteractionErrorCodeUnsupportedCluster     = 0xc3,
    // Gap in values is intentional.
    MatterInteractionErrorCodeNoUpstreamSubscription = 0xc5,
    MatterInteractionErrorCodeNeedsTimedInteraction  = 0xc6,
    MatterInteractionErrorCodeUnsupportedEvent       = 0xc7,
};
// clang-format on

NS_ASSUME_NONNULL_END
