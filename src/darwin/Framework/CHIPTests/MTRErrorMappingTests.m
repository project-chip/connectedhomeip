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

#import <Matter/Matter.h>
#import <XCTest/XCTest.h>

#import "MTRError_Test.h"

// A CHIP_ERROR-encoded integer that does not match any known mapping in
// MTRError errorForCHIPErrorCode:, so it round-trips through the default branch
// and produces an MTRErrorDomain Code=MTRErrorCodeGeneralError NSError -- the
// same shape upstream renders as "General error: 172" today.  Used to model
// the CHIP_ERROR_INTERNAL surfaced by the WiFiNetworkEnable commissioning step.
static const uint32_t kSomeUnknownCHIPErrorIntegerValue = 0xDEADBEEF;

// Spec-defined NetworkCommissioning cluster status enum values
// (see app-common/clusters/NetworkCommissioning/Enums.h). Mirrored as plain
// uint8_t literals because this test target intentionally cannot include
// CHIP-internal headers (see MTRError_Test.h: "we can't use CHIP_ERROR from
// there"). The values are pinned spec-side and cross-checked against the
// public MTRNetworkCommissioningStatus enum below.
static const uint8_t kNetworkCommissioningStatusSuccess = 0x00;
static const uint8_t kNetworkCommissioningStatusNetworkNotFound = 0x05;
static const uint8_t kNetworkCommissioningStatusAuthFailure = 0x07;

@interface MTRErrorMappingTests : XCTestCase
@end

@implementation MTRErrorMappingTests

- (void)testPublicNonInteractionAPIValues
{
    for (int errorCode = 1; errorCode <= MTRMaxErrorCode; errorCode++) {
        // A few error codes are not actually representing CHIP_ERROR values.
        if (errorCode == MTRErrorCodeWrongAddressType || errorCode == MTRErrorCodeUnknownSchema) {
            continue;
        }

        // All of these should round-trip appropriately.
        __auto_type * error = [NSError errorWithDomain:MTRErrorDomain code:errorCode userInfo:nil];
        __auto_type * newError1 = [MTRError errorWithCode:(MTRErrorCode) errorCode];
        XCTAssertEqual(newError1.domain, error.domain, "Testing error code %d", errorCode);
        XCTAssertEqual(newError1.code, error.code, "Testing error code %d", errorCode);

        __auto_type chipError = [MTRError errorToCHIPIntegerCode:error];
        __auto_type * newError2 = [MTRError errorForCHIPIntegerCode:chipError];
        XCTAssertEqual(newError2.domain, error.domain, "Testing error code %d", errorCode);
        XCTAssertEqual(newError2.code, error.code, "Testing error code %d", errorCode);
    }

    // Check that an unknown value becomes GeneralError.
    __auto_type * error = [MTRError errorWithCode:(MTRErrorCode) (MTRMaxErrorCode + 1)];
    XCTAssertEqual(error.domain, MTRErrorDomain);
    XCTAssertEqual(error.code, MTRMaxErrorCode + 1);

    __auto_type chipError = [MTRError errorToCHIPIntegerCode:error];
    __auto_type * newError = [MTRError errorForCHIPIntegerCode:chipError];
    XCTAssertEqual(newError.domain, MTRErrorDomain);
    XCTAssertEqual(newError.code, MTRErrorCodeGeneralError);
}

// Regression: ConnectNetwork response with NetworkCommissioningStatus=NetworkNotFound (5)
// must surface on the resulting NSError so commissioning UIs can render an
// "Incompatible Wi-Fi" message instead of a generic failure.
- (void)testConnectNetworkNetworkNotFoundProducesNetworkCommissioningStatusUserInfoKey
{
    NSError * error = [MTRError errorForCHIPIntegerCode:kSomeUnknownCHIPErrorIntegerValue
                          hasNetworkCommissioningStatus:YES
                             networkCommissioningStatus:kNetworkCommissioningStatusNetworkNotFound];

    // Backward-compat: domain/code remain MTRErrorDomain / MTRErrorCodeGeneralError.
    XCTAssertEqualObjects(error.domain, MTRErrorDomain);
    XCTAssertEqual(error.code, MTRErrorCodeGeneralError);

    // The new userInfo key carries the NetworkCommissioning cluster status byte.
    id status = error.userInfo[MTRErrorNetworkCommissioningStatusKey];
    XCTAssertNotNil(status);
    XCTAssertTrue([status isKindOfClass:[NSNumber class]]);
    XCTAssertEqual([status unsignedCharValue], kNetworkCommissioningStatusNetworkNotFound);
}

- (void)testConnectNetworkAuthFailurePropagatesStatus7
{
    NSError * error = [MTRError errorForCHIPIntegerCode:kSomeUnknownCHIPErrorIntegerValue
                          hasNetworkCommissioningStatus:YES
                             networkCommissioningStatus:kNetworkCommissioningStatusAuthFailure];

    id status = error.userInfo[MTRErrorNetworkCommissioningStatusKey];
    XCTAssertNotNil(status);
    XCTAssertEqual([status unsignedCharValue], kNetworkCommissioningStatusAuthFailure);
}

- (void)testNonNetworkCommissioningFailureHasNoNetworkStatusKey
{
    // Failure without any network-commissioning context must not carry the new
    // key, so consumers can rely on its presence as a signal that a
    // NetworkCommissioning status was actually reported.
    NSError * error = [MTRError errorForCHIPIntegerCode:kSomeUnknownCHIPErrorIntegerValue
                          hasNetworkCommissioningStatus:NO
                             networkCommissioningStatus:0];

    XCTAssertEqualObjects(error.domain, MTRErrorDomain);
    XCTAssertNil(error.userInfo[MTRErrorNetworkCommissioningStatusKey]);
}

- (void)testSuccessfulCommissioningStillProducesNilError
{
    // Regression guard: kSuccess (0) is not a failure, so the helper, when handed
    // CHIP_NO_ERROR, must still return nil regardless of the supplied status byte.
    NSError * error = [MTRError errorForCHIPIntegerCode:0 /* CHIP_NO_ERROR */
                          hasNetworkCommissioningStatus:YES
                             networkCommissioningStatus:kNetworkCommissioningStatusSuccess];
    XCTAssertNil(error);
}

// Coverage pin: every spec-defined NetworkCommissioning cluster status byte
// (0x00..0x0C, all 13 values) must propagate verbatim through the
// MTRErrorNetworkCommissioningStatusKey on the resulting NSError when paired
// with a non-success CHIP_ERROR. Catches accidental clamping, sign-extension,
// or enum-value-filtering regressions in the factory's userInfo plumbing --
// the factory must not interpret the status byte, only forward it.
- (void)testAllNetworkCommissioningStatusValuesRoundTripThroughUserInfo
{
    // Spec NetworkCommissioning::NetworkCommissioningStatusEnum, mirrored from
    // app-common/clusters/NetworkCommissioning/Enums.h. Listing literally so a
    // future spec addition forces an explicit update of this test rather than
    // silently slipping through.
    static const uint8_t kAllStatusBytes[] = {
        0x00, // kSuccess
        0x01, // kOutOfRange
        0x02, // kBoundsExceeded
        0x03, // kNetworkIDNotFound
        0x04, // kDuplicateNetworkID
        0x05, // kNetworkNotFound
        0x06, // kRegulatoryError
        0x07, // kAuthFailure
        0x08, // kUnsupportedSecurity
        0x09, // kOtherConnectionFailure
        0x0A, // kIPV6Failed
        0x0B, // kIPBindFailed
        0x0C, // kUnknownError
    };
    static const size_t kAllStatusByteCount = sizeof(kAllStatusBytes) / sizeof(kAllStatusBytes[0]);
    XCTAssertEqual(kAllStatusByteCount, 13u, @"Test must cover all 13 spec NetworkCommissioning status values");

    // Cross-check against the public MTRNetworkCommissioningStatus enum so
    // this test fails loudly if the spec adds a value and the public API
    // gains a slot for it but this list is not updated to match.
    XCTAssertEqual((uint8_t) MTRNetworkCommissioningStatusSuccess, kAllStatusBytes[0]);
    XCTAssertEqual((uint8_t) MTRNetworkCommissioningStatusOutOfRange, kAllStatusBytes[1]);
    XCTAssertEqual((uint8_t) MTRNetworkCommissioningStatusBoundsExceeded, kAllStatusBytes[2]);
    XCTAssertEqual((uint8_t) MTRNetworkCommissioningStatusNetworkIDNotFound, kAllStatusBytes[3]);
    XCTAssertEqual((uint8_t) MTRNetworkCommissioningStatusDuplicateNetworkID, kAllStatusBytes[4]);
    XCTAssertEqual((uint8_t) MTRNetworkCommissioningStatusNetworkNotFound, kAllStatusBytes[5]);
    XCTAssertEqual((uint8_t) MTRNetworkCommissioningStatusRegulatoryError, kAllStatusBytes[6]);
    XCTAssertEqual((uint8_t) MTRNetworkCommissioningStatusAuthFailure, kAllStatusBytes[7]);
    XCTAssertEqual((uint8_t) MTRNetworkCommissioningStatusUnsupportedSecurity, kAllStatusBytes[8]);
    XCTAssertEqual((uint8_t) MTRNetworkCommissioningStatusOtherConnectionFailure, kAllStatusBytes[9]);
    XCTAssertEqual((uint8_t) MTRNetworkCommissioningStatusIPV6Failed, kAllStatusBytes[10]);
    XCTAssertEqual((uint8_t) MTRNetworkCommissioningStatusIPBindFailed, kAllStatusBytes[11]);
    XCTAssertEqual((uint8_t) MTRNetworkCommissioningStatusUnknownError, kAllStatusBytes[12]);

    for (size_t i = 0; i < kAllStatusByteCount; i++) {
        uint8_t statusByte = kAllStatusBytes[i];
        NSError * error = [MTRError errorForCHIPIntegerCode:kSomeUnknownCHIPErrorIntegerValue
                              hasNetworkCommissioningStatus:YES
                                 networkCommissioningStatus:statusByte];

        XCTAssertNotNil(error, @"Status 0x%02x must produce a non-nil NSError when paired with a non-success CHIP_ERROR", statusByte);
        XCTAssertEqualObjects(error.domain, MTRErrorDomain, @"Status 0x%02x must use MTRErrorDomain", statusByte);
        XCTAssertEqual(error.code, MTRErrorCodeGeneralError, @"Status 0x%02x must round-trip the unknown CHIP_ERROR to GeneralError", statusByte);

        id status = error.userInfo[MTRErrorNetworkCommissioningStatusKey];
        XCTAssertNotNil(status, @"Status 0x%02x must surface MTRErrorNetworkCommissioningStatusKey", statusByte);
        XCTAssertTrue([status isKindOfClass:[NSNumber class]], @"Status 0x%02x must be boxed as NSNumber", statusByte);
        XCTAssertEqual([status unsignedCharValue], statusByte,
            @"Status byte 0x%02x must propagate verbatim into userInfo (got 0x%02x)",
            statusByte, [status unsignedCharValue]);
    }
}

// Negative-case coverage pin: the new userInfo key must be absent for every
// well-known MTRError code when the factory is invoked without a
// NetworkCommissioning status. Clients rely on key presence as the signal
// that a NetworkCommissioning status was actually reported by the device,
// so any leakage of the key onto unrelated errors would silently mis-fire
// commissioning UI branches (e.g. show "wrong Wi-Fi password" for a
// PASE-layer timeout). Broader than testNonNetworkCommissioningFailureHasNoNetworkStatusKey,
// which only exercises the GeneralError branch.
- (void)testNonNetworkCommissioningFailureNeverCarriesNetworkStatusKey
{
    // Mix of MTRError codes that have well-defined CHIP_ERROR integer
    // mappings (round-trip tested by testPublicNonInteractionAPIValues
    // above), plus the unknown integer that falls through to GeneralError.
    // The point is to exercise the userInfo plumbing across unrelated
    // CHIP_ERROR shapes (IM status, transport, integrity, timeout, generic)
    // so a regression where the key gets attached unconditionally would
    // surface here even if the GeneralError-only test passed.
    static const MTRErrorCode kNonNetworkCommissioningCodes[] = {
        MTRErrorCodeGeneralError,
        MTRErrorCodeInvalidState,
        MTRErrorCodeInvalidIntegerValue,
        MTRErrorCodeInvalidArgument,
        MTRErrorCodeInvalidMessageLength,
        MTRErrorCodeIntegrityCheckFailed,
        MTRErrorCodeTimeout,
        MTRErrorCodeBufferTooSmall,
        MTRErrorCodeFabricExists,
    };
    static const size_t kNonNetworkCommissioningCodeCount = sizeof(kNonNetworkCommissioningCodes) / sizeof(kNonNetworkCommissioningCodes[0]);

    for (size_t i = 0; i < kNonNetworkCommissioningCodeCount; i++) {
        MTRErrorCode code = kNonNetworkCommissioningCodes[i];

        // Round-trip the MTRError code into a CHIP integer so we exercise
        // the same factory path commissioning failures take, then deliver
        // it back through the network-aware overload with hasStatus:NO.
        NSError * shape = [NSError errorWithDomain:MTRErrorDomain code:code userInfo:nil];
        uint32_t chipInt = [MTRError errorToCHIPIntegerCode:shape];

        NSError * error = [MTRError errorForCHIPIntegerCode:chipInt
                              hasNetworkCommissioningStatus:NO
                                 networkCommissioningStatus:0];
        XCTAssertNotNil(error, @"Code %ld must produce a non-nil NSError", (long) code);
        XCTAssertEqualObjects(error.domain, MTRErrorDomain, @"Code %ld must use MTRErrorDomain", (long) code);
        XCTAssertNil(error.userInfo[MTRErrorNetworkCommissioningStatusKey],
            @"Code %ld must NOT carry MTRErrorNetworkCommissioningStatusKey when no NetworkCommissioning status was reported",
            (long) code);

        // Belt-and-suspenders: even if a caller mistakenly passes a non-zero
        // status byte while flagging hasStatus:NO, the byte must be ignored.
        // Pins that hasStatus is the gate, not "non-zero status byte".
        NSError * errorWithIgnoredByte = [MTRError errorForCHIPIntegerCode:chipInt
                                             hasNetworkCommissioningStatus:NO
                                                networkCommissioningStatus:kNetworkCommissioningStatusAuthFailure];
        XCTAssertNil(errorWithIgnoredByte.userInfo[MTRErrorNetworkCommissioningStatusKey],
            @"Code %ld with hasStatus=NO must ignore the supplied status byte", (long) code);
    }
}

// Regression: when the underlying CHIP_ERROR is an Interaction Model status
// (SdkPart=kIMGlobalStatus or kIMClusterStatus), the factory routes through
// +errorForIMStatus: and the resulting NSError lives in MTRInteractionErrorDomain.
// In that branch the NetworkCommissioning status byte MUST still be attached
// to userInfo so commissioning UIs can recover the reported NC status
// regardless of which NSError domain the underlying error rendered into --
// otherwise an AuthFailure surfaced via WiFiNetworkEnable IM status would be
// indistinguishable from a generic IM failure.
- (void)testIMStatusErrorWithNetworkCommissioningStatusForwardsTheStatusByte
{
    // SdkPart::kIMGlobalStatus = 5, code = 0x01 (Failure).
    // Layout: kRangeStartBit=24, kSdkPartStartBit=8, kSdkCodeStartBit=0; Range::kSDK=0.
    static const uint32_t kIMGlobalStatusFailureCHIPInteger = (0u << 24) | (5u << 8) | 0x01u; // 0x00000501
    // SdkPart::kIMClusterStatus = 6, code = 0x02 (cluster-specific status sample).
    static const uint32_t kIMClusterStatusSampleCHIPInteger = (0u << 24) | (6u << 8) | 0x02u; // 0x00000602

    static const uint32_t kIMIntegers[] = {
        kIMGlobalStatusFailureCHIPInteger,
        kIMClusterStatusSampleCHIPInteger,
    };
    static const size_t kIMIntegerCount = sizeof(kIMIntegers) / sizeof(kIMIntegers[0]);

    for (size_t i = 0; i < kIMIntegerCount; i++) {
        uint32_t imCHIPInteger = kIMIntegers[i];

        NSError * error = [MTRError errorForCHIPIntegerCode:imCHIPInteger
                              hasNetworkCommissioningStatus:YES
                                 networkCommissioningStatus:kNetworkCommissioningStatusNetworkNotFound];

        XCTAssertNotNil(error, @"IM-status CHIP integer 0x%08x must produce a non-nil NSError", imCHIPInteger);
        // IM-status errors are routed to MTRInteractionErrorDomain.
        XCTAssertEqualObjects(error.domain, MTRInteractionErrorDomain,
            @"IM-status CHIP integer 0x%08x must produce an MTRInteractionErrorDomain error", imCHIPInteger);
        // The status byte must be forwarded onto the IM-status error so
        // consumers can branch on it regardless of error domain.
        id status = error.userInfo[MTRErrorNetworkCommissioningStatusKey];
        XCTAssertNotNil(status,
            @"IM-status CHIP integer 0x%08x MUST carry MTRErrorNetworkCommissioningStatusKey "
            @"(presence is uniform across error domains).",
            imCHIPInteger);
        XCTAssertTrue([status isKindOfClass:[NSNumber class]]);
        XCTAssertEqual([(NSNumber *) status unsignedCharValue], kNetworkCommissioningStatusNetworkNotFound,
            @"IM-status CHIP integer 0x%08x must forward the status byte verbatim", imCHIPInteger);
    }
}

// Negative case: an IM-status CHIP_ERROR without a NetworkCommissioning
// status reported alongside it must NOT gain the key. Pins that the key
// presence on IM-domain errors is gated by the upstream signal, not added
// unconditionally to every IM error.
- (void)testIMStatusErrorWithoutNetworkCommissioningStatusOmitsTheKey
{
    static const uint32_t kIMGlobalStatusFailureCHIPInteger = (0u << 24) | (5u << 8) | 0x01u;
    NSError * error = [MTRError errorForCHIPIntegerCode:kIMGlobalStatusFailureCHIPInteger
                          hasNetworkCommissioningStatus:NO
                             networkCommissioningStatus:0];
    XCTAssertNotNil(error);
    XCTAssertEqualObjects(error.domain, MTRInteractionErrorDomain);
    XCTAssertNil(error.userInfo[MTRErrorNetworkCommissioningStatusKey],
        @"IM-status error without NC status must NOT carry MTRErrorNetworkCommissioningStatusKey");
}

// Regression: NetworkCommissioning status bytes outside the currently
// spec-defined 0x00..0x0C range (e.g. a future spec revision adds 0x0D, or a
// non-compliant accessory returns 0xFE/0xFF) must still propagate verbatim --
// the framework's job is to forward, not to interpret. Catches a future
// "validate and zero unknown bytes" or "clamp to MTRNetworkCommissioningStatusUnknownError"
// regression that would silently hide accessory-side bugs from commissioning UIs.
//
// Complements testAllNetworkCommissioningStatusValuesRoundTripThroughUserInfo,
// which only exercises spec-defined values; this test exercises forward-compat.
- (void)testOutOfSpecNetworkCommissioningStatusBytesForwardVerbatim
{
    // Pick bytes that are intentionally outside the current spec enum range.
    // 0x0D is the immediate next spec slot; 0x7F is mid-range; 0xFE / 0xFF
    // exercise high bits (catches accidental sign-extension to int8_t).
    static const uint8_t kOutOfSpecBytes[] = { 0x0D, 0x10, 0x7F, 0x80, 0xFE, 0xFF };
    static const size_t kOutOfSpecCount = sizeof(kOutOfSpecBytes) / sizeof(kOutOfSpecBytes[0]);

    for (size_t i = 0; i < kOutOfSpecCount; i++) {
        uint8_t statusByte = kOutOfSpecBytes[i];
        NSError * error = [MTRError errorForCHIPIntegerCode:kSomeUnknownCHIPErrorIntegerValue
                              hasNetworkCommissioningStatus:YES
                                 networkCommissioningStatus:statusByte];

        XCTAssertNotNil(error, @"Out-of-spec status 0x%02x must still produce an NSError", statusByte);
        XCTAssertEqualObjects(error.domain, MTRErrorDomain,
            @"Out-of-spec status 0x%02x must use MTRErrorDomain", statusByte);
        XCTAssertEqual(error.code, MTRErrorCodeGeneralError,
            @"Out-of-spec status 0x%02x must round-trip the unknown CHIP_ERROR to GeneralError", statusByte);

        id status = error.userInfo[MTRErrorNetworkCommissioningStatusKey];
        XCTAssertNotNil(status,
            @"Out-of-spec status 0x%02x must still surface MTRErrorNetworkCommissioningStatusKey "
            @"(framework forwards, does not validate)",
            statusByte);
        XCTAssertTrue([status isKindOfClass:[NSNumber class]],
            @"Out-of-spec status 0x%02x must be boxed as NSNumber", statusByte);
        NSNumber * statusNumber = (NSNumber *) status;
        XCTAssertEqual([statusNumber unsignedCharValue], statusByte,
            @"Out-of-spec status byte 0x%02x must propagate verbatim into userInfo (got 0x%02x); "
            @"the framework must not clamp, mask, or sign-extend it",
            statusByte, [statusNumber unsignedCharValue]);
        // Pin that the high-bit bytes did not get sign-extended into a large
        // signed integer (a classic int8_t -> int -> NSNumber bug).
        XCTAssertEqual([statusNumber unsignedIntegerValue], (NSUInteger) statusByte,
            @"Out-of-spec status 0x%02x must NOT sign-extend (unsignedIntegerValue mismatch)", statusByte);
        XCTAssertEqual([statusNumber intValue], (int) statusByte,
            @"Out-of-spec status 0x%02x must round-trip through -intValue without sign-extension", statusByte);
    }
}

// Regression: pin the public consumer contract for
// MTRErrorNetworkCommissioningStatusKey -- both the literal string value
// (which ships in NSError userInfo dictionaries that may be serialized,
// archived, or logged by clients) and the equality contract between the
// boxed NSNumber and the public MTRNetworkCommissioningStatus enum slots
// that commissioning UIs branch on.
//
// Without this, a "rename the userInfo key" refactor or a "switch the boxed
// type to NSString/NSData" refactor would silently break every client that
// switches on the status, including the actionable UI strings called out in
// the PR description ("the accessory could not find the Wi-Fi network --
// try a 2.4 GHz network or check the SSID and password").
- (void)testNetworkCommissioningStatusKeyConstantAndNSNumberConsumerContract
{
    // The literal key string is part of the ABI: it appears in archived
    // NSError userInfo dictionaries and in client-side string-keyed lookups
    // that do not link against the symbol. If this assertion fires, the
    // key was renamed -- bump the doc & coordinate with consumers.
    XCTAssertEqualObjects(MTRErrorNetworkCommissioningStatusKey, @"MTRNetworkCommissioningStatus",
        @"MTRErrorNetworkCommissioningStatusKey is part of the public NSError userInfo ABI; "
        @"renaming silently breaks every consumer that reads userInfo by literal string.");

    // String-keyed lookup (the path used by clients that don't have the
    // header) must return the same object as symbol-keyed lookup.
    NSError * error = [MTRError errorForCHIPIntegerCode:kSomeUnknownCHIPErrorIntegerValue
                          hasNetworkCommissioningStatus:YES
                             networkCommissioningStatus:kNetworkCommissioningStatusAuthFailure];
    id viaSymbol = error.userInfo[MTRErrorNetworkCommissioningStatusKey];
    id viaLiteral = error.userInfo[@"MTRNetworkCommissioningStatus"];
    XCTAssertNotNil(viaSymbol);
    XCTAssertNotNil(viaLiteral);
    XCTAssertEqualObjects(viaSymbol, viaLiteral,
        @"Symbol-keyed and literal-string-keyed userInfo lookups must return the same value.");

    // The boxed value must compare equal to the public MTRNetworkCommissioningStatus
    // enum slot -- this is the assertion commissioning UIs will actually write
    // (e.g. `if ([status integerValue] == MTRNetworkCommissioningStatusAuthFailure]`).
    // Catches a regression where the byte is boxed as a different numeric type
    // (e.g. int vs unsigned char) such that -[NSNumber isEqualToNumber:] still
    // works but -integerValue / -unsignedIntegerValue drift.
    XCTAssertTrue([viaSymbol isKindOfClass:[NSNumber class]]);
    NSNumber * viaSymbolNumber = (NSNumber *) viaSymbol;
    XCTAssertEqualObjects(viaSymbolNumber, @(MTRNetworkCommissioningStatusAuthFailure),
        @"Boxed status must NSNumber-compare equal to the matching MTRNetworkCommissioningStatus enum slot.");
    XCTAssertEqual([viaSymbolNumber integerValue], (NSInteger) MTRNetworkCommissioningStatusAuthFailure,
        @"-integerValue must equal the matching MTRNetworkCommissioningStatus enum slot.");
    XCTAssertEqual([viaSymbolNumber unsignedIntegerValue], (NSUInteger) MTRNetworkCommissioningStatusAuthFailure,
        @"-unsignedIntegerValue must equal the matching MTRNetworkCommissioningStatus enum slot.");

    // Pin that the value is suitable for plist / JSON serialization (clients
    // may archive NSError userInfo dictionaries). NSNumber satisfies
    // NSSecureCoding and JSON-encodes as a number.
    XCTAssertTrue([(NSNumber *) viaSymbol conformsToProtocol:@protocol(NSSecureCoding)]);
    XCTAssertTrue([NSJSONSerialization isValidJSONObject:@{ MTRErrorNetworkCommissioningStatusKey : viaSymbol }],
        @"The userInfo entry must remain JSON-serializable so clients can log it verbatim.");
}

// Regression: the GeneralError default branch in errorForCHIPErrorCode: writes
// its own additionalUserInfo entries (specifically @"errorCode" carrying the
// raw CHIP_ERROR integer, plus NSLocalizedDescriptionKey "General error: N").
// When a NetworkCommissioning status is layered on top, the merge must
// PRESERVE the default-branch entries -- a naive assignment would clobber
// them and break consumers that read either userInfo[@"errorCode"] (e.g. for
// crash-log triage) or NSError.localizedDescription (e.g. log lines and UI).
- (void)testNetworkCommissioningStatusMergesWithDefaultBranchUserInfoWithoutClobbering
{
    NSError * error = [MTRError errorForCHIPIntegerCode:kSomeUnknownCHIPErrorIntegerValue
                          hasNetworkCommissioningStatus:YES
                             networkCommissioningStatus:kNetworkCommissioningStatusNetworkNotFound];
    XCTAssertNotNil(error);
    XCTAssertEqualObjects(error.domain, MTRErrorDomain);
    XCTAssertEqual(error.code, MTRErrorCodeGeneralError);

    // Default-branch entries must survive the merge.
    id rawErrorCode = error.userInfo[@"errorCode"];
    XCTAssertNotNil(rawErrorCode, @"GeneralError must continue to expose the raw CHIP integer under userInfo[@\"errorCode\"]");
    XCTAssertTrue([rawErrorCode isKindOfClass:[NSNumber class]]);
    XCTAssertEqual([(NSNumber *) rawErrorCode unsignedIntValue], kSomeUnknownCHIPErrorIntegerValue,
        @"errorCode userInfo must equal the original CHIP integer (got %@)", rawErrorCode);

    NSString * description = error.userInfo[NSLocalizedDescriptionKey];
    XCTAssertNotNil(description, @"NSLocalizedDescriptionKey must not be wiped when status is added");
    XCTAssertTrue([description containsString:@"General error"],
        @"localizedDescription must still describe the error (got %@)", description);

    // The status itself must also be present alongside the default-branch keys.
    XCTAssertEqualObjects(error.userInfo[MTRErrorNetworkCommissioningStatusKey],
        @(kNetworkCommissioningStatusNetworkNotFound));

    // -[NSError localizedDescription] reads NSLocalizedDescriptionKey; pin
    // that the public accessor still works (catches a regression where the
    // merge accidentally dropped the description entry).
    XCTAssertNotNil(error.localizedDescription);
    XCTAssertTrue([error.localizedDescription containsString:@"General error"],
        @"-localizedDescription must reflect the description entry");
}

// Regression: for recognized-CHIP-error codes (e.g. CHIP_ERROR_TIMEOUT ->
// MTRErrorCodeTimeout), userInfo only carries NSLocalizedDescriptionKey on
// the path before the status merge. Pin that the merge keeps the localized
// description intact and adds the status key, even though there is no
// additionalUserInfo dictionary on this path. Catches a "merge order"
// regression where the new key replaces userInfo wholesale instead of
// extending it. Also pins that the recognized-error path does NOT gain a
// stray @"errorCode" entry (that belongs to the GeneralError branch only).
- (void)testNetworkCommissioningStatusOnRecognizedErrorPreservesLocalizedDescription
{
    // Build a recognized MTRErrorCodeTimeout NSError, round-trip it to its
    // CHIP integer, then re-render through the network-aware overload.
    NSError * shape = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeTimeout userInfo:nil];
    uint32_t timeoutCHIPInteger = [MTRError errorToCHIPIntegerCode:shape];

    NSError * error = [MTRError errorForCHIPIntegerCode:timeoutCHIPInteger
                          hasNetworkCommissioningStatus:YES
                             networkCommissioningStatus:kNetworkCommissioningStatusAuthFailure];
    XCTAssertNotNil(error);
    XCTAssertEqualObjects(error.domain, MTRErrorDomain);
    XCTAssertEqual(error.code, MTRErrorCodeTimeout,
        @"Recognized CHIP_ERROR_TIMEOUT must still map to MTRErrorCodeTimeout when status is layered on");

    // Status added.
    XCTAssertEqualObjects(error.userInfo[MTRErrorNetworkCommissioningStatusKey],
        @(kNetworkCommissioningStatusAuthFailure));

    // Localized description preserved -- not clobbered by the status merge.
    NSString * description = error.userInfo[NSLocalizedDescriptionKey];
    XCTAssertNotNil(description, @"Recognized-error path must keep NSLocalizedDescriptionKey after status merge");
    XCTAssertEqualObjects(description, error.localizedDescription);
    // Recognized-Timeout path must NOT carry a stray @"errorCode" entry.
    XCTAssertNil(error.userInfo[@"errorCode"],
        @"Recognized-error path must not gain a default-branch @\"errorCode\" entry via the merge");
}

// Regression: errorToCHIPIntegerCode reads the original CHIP_ERROR via the
// MTRErrorHolder associated object, NOT via NSError.userInfo. Pin that adding
// the new MTRErrorNetworkCommissioningStatusKey to userInfo does not perturb
// the CHIP_ERROR <-> NSError round-trip. Catches a regression where the
// status-adding code path forgets to associate the holder (e.g. by rebuilding
// the NSError after the holder attach instead of before).
- (void)testNetworkCommissioningStatusDoesNotBreakCHIPIntegerRoundTrip
{
    NSError * errorWithStatus = [MTRError errorForCHIPIntegerCode:kSomeUnknownCHIPErrorIntegerValue
                                    hasNetworkCommissioningStatus:YES
                                       networkCommissioningStatus:kNetworkCommissioningStatusAuthFailure];
    NSError * errorWithoutStatus = [MTRError errorForCHIPIntegerCode:kSomeUnknownCHIPErrorIntegerValue
                                       hasNetworkCommissioningStatus:NO
                                          networkCommissioningStatus:0];

    // Both NSErrors must round-trip back to the same CHIP integer they came
    // from -- the status key in userInfo is purely additive metadata.
    uint32_t roundTripWithStatus = [MTRError errorToCHIPIntegerCode:errorWithStatus];
    uint32_t roundTripWithoutStatus = [MTRError errorToCHIPIntegerCode:errorWithoutStatus];
    XCTAssertEqual(roundTripWithStatus, kSomeUnknownCHIPErrorIntegerValue,
        @"errorToCHIPIntegerCode must recover the original CHIP integer from a status-bearing NSError (got 0x%08x)",
        roundTripWithStatus);
    XCTAssertEqual(roundTripWithoutStatus, kSomeUnknownCHIPErrorIntegerValue);
    XCTAssertEqual(roundTripWithStatus, roundTripWithoutStatus,
        @"Round-tripped CHIP integer must not depend on whether the NetworkCommissioning status was attached");

    // Calling the factory repeatedly with the same inputs must produce
    // independently-correct NSErrors. Pin that there is no shared/mutable
    // state across calls (e.g. a cached userInfo dict that gets mutated).
    NSError * a = [MTRError errorForCHIPIntegerCode:kSomeUnknownCHIPErrorIntegerValue
                      hasNetworkCommissioningStatus:YES
                         networkCommissioningStatus:kNetworkCommissioningStatusNetworkNotFound];
    NSError * b = [MTRError errorForCHIPIntegerCode:kSomeUnknownCHIPErrorIntegerValue
                      hasNetworkCommissioningStatus:YES
                         networkCommissioningStatus:kNetworkCommissioningStatusAuthFailure];
    XCTAssertEqualObjects(a.userInfo[MTRErrorNetworkCommissioningStatusKey],
        @(kNetworkCommissioningStatusNetworkNotFound),
        @"First NSError must retain its own status byte even after a second call with a different byte");
    XCTAssertEqualObjects(b.userInfo[MTRErrorNetworkCommissioningStatusKey],
        @(kNetworkCommissioningStatusAuthFailure));
}

// Regression: a status byte of 0 (kSuccess) explicitly delivered alongside a
// non-success CHIP_ERROR is anomalous (the device reported "the network
// commissioning step succeeded" while upstream still surfaced an error) but
// MUST be forwarded verbatim to the client, because consumers branch on key
// presence to decide whether to treat the failure as network-related, and
// dropping a 0 byte would silently mis-route the failure.
//
// Key contract: the userInfo value is @(0), NOT nil and NOT absent.
// Catches a regression where a "if (status != 0)" guard sneaks in and treats
// 0 as "no status reported".
- (void)testNetworkCommissioningStatusZeroIsForwardedNotTreatedAsAbsent
{
    NSError * error = [MTRError errorForCHIPIntegerCode:kSomeUnknownCHIPErrorIntegerValue
                          hasNetworkCommissioningStatus:YES
                             networkCommissioningStatus:kNetworkCommissioningStatusSuccess];
    XCTAssertNotNil(error, @"A non-success CHIP_ERROR must still produce an NSError when the status byte is 0");

    id status = error.userInfo[MTRErrorNetworkCommissioningStatusKey];
    XCTAssertNotNil(status,
        @"hasStatus=YES with status byte 0 must still surface MTRErrorNetworkCommissioningStatusKey "
        @"(consumers branch on key presence, not on byte value)");
    XCTAssertTrue([status isKindOfClass:[NSNumber class]]);
    XCTAssertEqualObjects(status, @(0),
        @"Status byte 0 (kSuccess) must be boxed as @(0), not nil and not omitted");
    // Belt-and-suspenders: NSDictionary lookup distinguishes @(0) from absent
    // only via -objectForKey: returning non-nil. Pin that the dictionary
    // actually contains the key.
    XCTAssertTrue([error.userInfo.allKeys containsObject:MTRErrorNetworkCommissioningStatusKey],
        @"userInfo must literally contain the key, not merely happen to return nil/zero on lookup");
    XCTAssertEqual([(NSNumber *) status unsignedCharValue], (uint8_t) 0);
}

@end
