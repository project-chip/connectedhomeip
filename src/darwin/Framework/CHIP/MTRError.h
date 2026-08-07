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

#import <Matter/MTRDefines.h>

NS_ASSUME_NONNULL_BEGIN

MTR_EXTERN NSErrorDomain const MTRErrorDomain MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
MTR_EXTERN NSErrorDomain const MTRInteractionErrorDomain MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

/**
 * NSNumber userInfo key carrying the numeric attestation-verification result for an
 * MTRErrorCodeIntegrityCheckFailed error emitted from device-attestation verification.
 * Lets callers distinguish, e.g., attestation-nonce mismatch (502) from PAA-not-found
 * (101) without otherwise being collapsed to a single integrity-check failure.
 *
 * The integer values match the underlying Matter SDK attestation-verification-result enum
 * and are intended for triage / structured discrimination; specific numeric values are not
 * stable across releases.
 */
MTR_EXTERN NSErrorUserInfoKey const MTRAttestationVerificationResultKey MTR_PROVISIONALLY_AVAILABLE;

/**
 * NSNumber userInfo keys carrying the vendor ID and product ID the device asserts in its
 * BasicInformation cluster, captured at the time of attestation verification. Present in
 * NSError userInfo for attestation-related failures so callers can identify which device's
 * chain failed validation without walking the attestation-device-info struct themselves.
 *
 * These values are exactly what the device claims in its BasicInformation cluster — there
 * is no guarantee the device is certified. In fact, one of the possible attestation-failure
 * modes is that these BasicInformation values do not match the IDs in the device's
 * certification declaration. Useful for triage and incident correlation, but should not
 * be trusted as authoritative identity until attestation succeeds.
 */
MTR_EXTERN NSErrorUserInfoKey const MTRDeviceBasicInformationVendorIDKey MTR_PROVISIONALLY_AVAILABLE;
MTR_EXTERN NSErrorUserInfoKey const MTRDeviceBasicInformationProductIDKey MTR_PROVISIONALLY_AVAILABLE;

/**
 * NSNumber userInfo key carrying the raw 32-bit underlying-error integer backing this NSError
 * when it was bridged from an underlying Matter SDK error. Present on every error in
 * MTRErrorDomain produced by the error bridge.
 *
 * The string value associated with this constant — i.e. the literal NSString returned by
 * this NSErrorUserInfoKey when used as a dictionary key — is @"errorCode". That is the
 * same literal key documented for MTRErrorCodeGeneralError below, so a caller reading
 * userInfo[MTRUnderlyingErrorCodeKey] sees exactly the same NSNumber as a caller reading
 * userInfo[@"errorCode"].
 *
 * This value is always populated by the bridge when an NSError in MTRErrorDomain is produced
 * from a Matter SDK error, and is not overridable by internal callers — it is the
 * authoritative integer underlying the bridged NSError.
 *
 * Intended for triage and log correlation; specific integer values are not stable across
 * releases.
 */
MTR_EXTERN NSErrorUserInfoKey const MTRUnderlyingErrorCodeKey MTR_PROVISIONALLY_AVAILABLE;

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
typedef NS_ERROR_ENUM(MTRErrorDomain, MTRErrorCode) {
    /**
     * MTRErrorCodeGeneralError represents a generic Matter error with no
     * further categorization.
     *
     * The userInfo will have MTRUnderlyingErrorCodeKey (string value
     * @"errorCode") populated, whose value will be an NSNumber representing
     * the underlying Matter error code.  These integer values should not be
     * assumed to be stable across releases, but may be useful in logging and
     * debugging.
     *
     * Note: as of this release, MTRUnderlyingErrorCodeKey is present on
     * EVERY error in MTRErrorDomain produced by the bridge, not just
     * MTRErrorCodeGeneralError. Do not use the presence of this key as a
     * discriminator for GeneralError; check error.code ==
     * MTRErrorCodeGeneralError instead.
     */
    MTRErrorCodeGeneralError = 1,
    MTRErrorCodeInvalidStringLength = 2,
    MTRErrorCodeInvalidIntegerValue = 3,
    MTRErrorCodeInvalidArgument = 4,
    MTRErrorCodeInvalidMessageLength = 5,
    MTRErrorCodeInvalidState = 6,
    MTRErrorCodeWrongAddressType = 7,
    MTRErrorCodeIntegrityCheckFailed = 8,
    MTRErrorCodeTimeout = 9,
    MTRErrorCodeBufferTooSmall = 10,

    /**
     * MTRErrorCodeFabricExists is returned when trying to commission a device
     * into a fabric when it's already part of that fabric.
     */
    MTRErrorCodeFabricExists = 11,

    /**
     * MTRErrorCodeUnknownSchema means the schema for the given cluster/attribute,
     * cluster/event, or cluster/command combination is not known.
     */
    MTRErrorCodeUnknownSchema MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0)) = 12,

    /**
     * MTRErrorCodeSchemaMismatch means that provided data did not match the
     * expected schema.
     */
    MTRErrorCodeSchemaMismatch MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0)) = 13,

    /**
     * MTRErrorCodeTLVDecodeFailed means that the TLV being decoded was malformed in
     * some way.  This can include things like lengths running past the end of
     * the buffer, strings that are not actually UTF-8, and various other
     * TLV-level failures.
     */
    MTRErrorCodeTLVDecodeFailed MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0)) = 14,

    /**
     * MTRErrorCodeDNSSDUnauthorized means that the application is not
     * authorized to perform DNS_SD lookups.  This typically means missing
     * entries for "_matter._tcp" (for operational lookup) and "_matterc._udp"
     * (for commissionable lookup) under the NSBonjourServices key in the
     * application's Info.plist.
     */
    MTRErrorCodeDNSSDUnauthorized MTR_AVAILABLE(ios(17.2), macos(14.2), watchos(10.2), tvos(17.2)) = 15,

    /**
     * The operation was cancelled.
     */
    MTRErrorCodeCancelled MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6)) = 16,

    /**
     * Access to some resource was denied.
     */
    MTRErrorCodeAccessDenied MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4)) = 17,

    /**
     * A request was made to some entity, and that entity cannot handle the
     * request right now, but might be able to at a different point in time.
     */
    MTRErrorCodeBusy MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4)) = 18,

    /**
     * Something was requested that could not be located.
     */
    MTRErrorCodeNotFound MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4)) = 19,
} MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

#define MTRMaxErrorCode MTRErrorCodeNotFound

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
typedef NS_ERROR_ENUM(MTRInteractionErrorDomain, MTRInteractionErrorCode) {
    // These values come from the general status code table in the Matter
    // Interaction Model specification.
    MTRInteractionErrorCodeFailure = 0x01,
    MTRInteractionErrorCodeInvalidSubscription = 0x7d,
    MTRInteractionErrorCodeUnsupportedAccess = 0x7e,
    MTRInteractionErrorCodeUnsupportedEndpoint = 0x7f,
    MTRInteractionErrorCodeInvalidAction = 0x80,
    MTRInteractionErrorCodeUnsupportedCommand = 0x81,
    MTRInteractionErrorCodeInvalidCommand = 0x85,
    MTRInteractionErrorCodeUnsupportedAttribute = 0x86,
    MTRInteractionErrorCodeConstraintError = 0x87,
    MTRInteractionErrorCodeUnsupportedWrite = 0x88,
    MTRInteractionErrorCodeResourceExhausted = 0x89,
    MTRInteractionErrorCodeNotFound = 0x8b,
    MTRInteractionErrorCodeUnreportableAttribute = 0x8c,
    MTRInteractionErrorCodeInvalidDataType = 0x8d,
    MTRInteractionErrorCodeUnsupportedRead = 0x8f,
    MTRInteractionErrorCodeDataVersionMismatch = 0x92,
    MTRInteractionErrorCodeTimeout = 0x94,
    MTRInteractionErrorCodeBusy = 0x9c,
    MTRInteractionErrorCodeAccessRestricted MTR_AVAILABLE(ios(26.0), macos(26.0), watchos(26.0), tvos(26.0)) = 0x9d,
    MTRInteractionErrorCodeUnsupportedCluster = 0xc3,
    MTRInteractionErrorCodeNoUpstreamSubscription = 0xc5,
    MTRInteractionErrorCodeNeedsTimedInteraction = 0xc6,
    MTRInteractionErrorCodeUnsupportedEvent = 0xc7,
    MTRInteractionErrorCodePathsExhausted = 0xc8,
    MTRInteractionErrorCodeTimedRequestMismatch = 0xc9,
    MTRInteractionErrorCodeFailsafeRequired = 0xca,
    MTRInteractionErrorCodeInvalidInState MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6)) = 0xcb,
    MTRInteractionErrorCodeNoCommandResponse MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6)) = 0xcc,
    MTRInteractionErrorCodeDynamicConstraintError MTR_PROVISIONALLY_AVAILABLE = 0xcf,
    MTRInteractionErrorCodeInvalidTransportType MTR_PROVISIONALLY_AVAILABLE = 0xd1,
} MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

/**
 * Accessors for source-location information that may be attached to NSError objects
 * bridged from the underlying Matter error type. The information is available when the
 * underlying SDK was built to record source locations; otherwise the accessors return
 * nil / 0.
 *
 * Lifetime caveat: these accessors are backed by an associated object attached to the
 * NSError instance at bridge time. Associated objects are not preserved by NSCoding or
 * NSCopying — if the NSError is archived, unarchived, or copied through any mechanism
 * that does not propagate associated objects (NSKeyedArchiver, [error copy], crossing
 * an XPC boundary, etc.), these accessors will return nil / 0 on the resulting object
 * even when the original would have returned a value.
 *
 * For MTRInteractionErrorDomain errors the source location reflects the StatusIB
 * encoding site inside the Matter SDK, not the device-reported origin of the status.
 */
@interface NSError (Matter)

/**
 * The source-file basename where the underlying Matter error was originally created.
 * Returns nil when the SDK did not record a source location, when the error was not
 * bridged from a Matter error, or when no source string was captured.
 *
 * Only the basename is exposed — the full path is intentionally elided to avoid leaking
 * build-host paths. Intended for triage; not stable across releases.
 */
@property (nonatomic, readonly, nullable) NSString * mtr_underlyingMatterErrorSourceFile MTR_PROVISIONALLY_AVAILABLE;

/**
 * The source-line where the underlying Matter error was originally created. Returns 0
 * when not available.
 */
@property (nonatomic, readonly) NSUInteger mtr_underlyingMatterErrorSourceLine MTR_PROVISIONALLY_AVAILABLE;

@end

NS_ASSUME_NONNULL_END
