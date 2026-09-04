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

// NOTE: This is a plain .m file (not .mm). It exercises the MTRError bridge entirely through
// the integer-code test API in MTRError_Test.h (errorForCHIPIntegerCode:/errorToCHIPIntegerCode:),
// which deliberately avoids exposing CHIP_ERROR so these tests do not need to compile as
// Objective-C++.

#import "MTRError_Test.h"

@interface MTRErrorMappingTests : XCTestCase
@end

@implementation MTRErrorMappingTests

- (void)testUnderlyingErrorCodeKeyPresentOnNonGeneralErrorCode
{
    // The doc-comment contract on MTRErrorCodeGeneralError used to imply that
    // userInfo[MTRUnderlyingErrorCodeKey] (string @"errorCode") was a discriminator for
    // GeneralError. As of this release, the key is populated on EVERY bridged error in
    // MTRErrorDomain — including specific MTRErrorCode values like Timeout. Pin that contract
    // here so that code that (incorrectly) used presence of the key as a GeneralError
    // discriminator at least triggers a test failure if anyone reverts the change.
    NSError * timeoutBridged = [MTRError errorForCHIPIntegerCode:0x00000032u /* CHIP_ERROR_TIMEOUT */];
    XCTAssertEqualObjects(timeoutBridged.domain, MTRErrorDomain);
    XCTAssertEqual((NSInteger) timeoutBridged.code, MTRErrorCodeTimeout,
        @"CHIP_ERROR_TIMEOUT must map to the specific MTRErrorCodeTimeout (not GeneralError)");
    XCTAssertNotNil(timeoutBridged.userInfo[MTRUnderlyingErrorCodeKey],
        @"MTRUnderlyingErrorCodeKey must be present on EVERY bridged MTRErrorDomain error, "
        @"not just MTRErrorCodeGeneralError. Presence cannot be used as a GeneralError discriminator.");
}

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

- (void)testUnderlyingErrorCodeKeyPopulatedOnBridgedErrors
{
    // Pick one CHIP_ERROR that has no specific MTRErrorCode mapping (so the bridge falls back
    // to MTRErrorCodeGeneralError) plus one that does map to a specific MTRErrorCode
    // (CHIP_ERROR_INVALID_ARGUMENT -> MTRErrorCodeInvalidArgument). The userInfo key must be
    // populated on every bridged error regardless of which MTRErrorCode the bridge selected.
    const uint32_t kChipCodes[] = {
        0x0000000Bu, // CHIP_ERROR_NO_MEMORY (Core sdkPart, code 0x0B)
        0x0000002Fu, // CHIP_ERROR_INVALID_ARGUMENT (Core sdkPart, code 0x2F)
    };

    for (size_t i = 0; i < sizeof(kChipCodes) / sizeof(kChipCodes[0]); ++i) {
        uint32_t chipCode = kChipCodes[i];
        NSError * bridged = [MTRError errorForCHIPIntegerCode:chipCode];
        XCTAssertEqualObjects(bridged.domain, MTRErrorDomain);

        id underlying = bridged.userInfo[MTRUnderlyingErrorCodeKey];
        XCTAssertTrue([underlying isKindOfClass:[NSNumber class]],
            @"MTRUnderlyingErrorCodeKey missing or wrong type for chip code 0x%08X", chipCode);
        XCTAssertEqual([(NSNumber *) underlying unsignedIntValue], chipCode,
            @"MTRUnderlyingErrorCodeKey value mismatch for chip code 0x%08X", chipCode);

        // Round-trip back to the underlying integer.
        XCTAssertEqual([MTRError errorToCHIPIntegerCode:bridged], chipCode,
            @"errorToCHIPIntegerCode: did not round-trip for chip code 0x%08X", chipCode);
    }
}

- (void)testPublicUserInfoKeyConstantsHaveStableStringValues
{
    // These string values are part of MTRError's documented userInfo surface — code outside
    // the framework can read them. Pinning them in a test guards against
    // accidental renames that would silently break consumers.
    XCTAssertEqualObjects(MTRUnderlyingErrorCodeKey, @"errorCode");
    XCTAssertEqualObjects(MTRAttestationVerificationResultKey, @"attestationVerificationResult");
    XCTAssertEqualObjects(MTRDeviceBasicInformationVendorIDKey, @"deviceBasicInformationVendorID");
    XCTAssertEqualObjects(MTRDeviceBasicInformationProductIDKey, @"deviceBasicInformationProductID");
}

- (void)testUnderlyingErrorCodeKeyMatchesLegacyDocumentedKeyForGeneralError
{
    // MTRErrorCodeGeneralError historically surfaced @"errorCode" in userInfo. The new
    // MTRUnderlyingErrorCodeKey constant must resolve to the same string so existing
    // code reading userInfo[@"errorCode"] keeps working unchanged.
    XCTAssertEqualObjects(MTRUnderlyingErrorCodeKey, @"errorCode");

    // Concretely: bridge a CHIP error that has no specific MTRErrorCode mapping (so we land in
    // the GeneralError fallback) and confirm both spellings return the same NSNumber.
    NSError * bridged = [MTRError errorForCHIPIntegerCode:0x0000000Bu /* CHIP_ERROR_NO_MEMORY */];
    XCTAssertEqual(bridged.code, MTRErrorCodeGeneralError);
    id viaConstant = bridged.userInfo[MTRUnderlyingErrorCodeKey];
    id viaLegacyString = bridged.userInfo[@"errorCode"];
    XCTAssertNotNil(viaConstant);
    XCTAssertEqualObjects(viaConstant, viaLegacyString);
}

- (void)testAdditionalUserInfoMergedOnIMStatusBridge
{
    // CHIP_ERROR encoding: (Range::kSDK << 24) | (SdkPart::kIMGlobalStatus << 8) | code.
    // SdkPart::kIMGlobalStatus is 5; Status::Failure = 0x01. So this is an IM-status error
    // that takes the IsIMStatus() early-return branch in errorForCHIPErrorCode:.
    const uint32_t kImStatusFailure = (5u << 8) | 0x01u; // 0x00000501

    // Without additionalUserInfo, the bridged error lands in MTRInteractionErrorDomain.
    NSError * plain = [MTRError errorForCHIPIntegerCode:kImStatusFailure];
    XCTAssertEqualObjects(plain.domain, MTRInteractionErrorDomain);

    // With additionalUserInfo, the IM-status path must merge it in (regression guard for
    // copilot's catch — the early return previously dropped additionalUserInfo silently).
    NSDictionary * extras = @{
        @"customKey1" : @"customValue1",
        @"customKey2" : @42,
    };
    NSError * merged = [MTRError errorForCHIPIntegerCode:kImStatusFailure additionalUserInfo:extras];
    XCTAssertEqualObjects(merged.domain, MTRInteractionErrorDomain);
    XCTAssertEqual(merged.code, plain.code, @"IM-status merge must preserve the original code");
    XCTAssertEqualObjects(merged.userInfo[@"customKey1"], @"customValue1");
    XCTAssertEqualObjects(merged.userInfo[@"customKey2"], @42);
    // Original IM-status userInfo entries (description) must survive the merge.
    XCTAssertNotNil(merged.userInfo[NSLocalizedDescriptionKey]);
}

- (void)testAdditionalUserInfoMergedOnCoreErrorBridge
{
    // Same regression guard for the non-IM-status path: additionalUserInfo must be merged
    // into the resulting NSError alongside the bridge's own keys (NSLocalizedDescriptionKey,
    // MTRUnderlyingErrorCodeKey, etc).
    NSDictionary * extras = @{ @"someAttestationKey" : @"someValue" };
    NSError * bridged = [MTRError errorForCHIPIntegerCode:0x0000000Bu /* CHIP_ERROR_NO_MEMORY */
                                       additionalUserInfo:extras];
    XCTAssertEqualObjects(bridged.domain, MTRErrorDomain);
    XCTAssertEqualObjects(bridged.userInfo[@"someAttestationKey"], @"someValue");
    XCTAssertNotNil(bridged.userInfo[MTRUnderlyingErrorCodeKey]);
    XCTAssertNotNil(bridged.userInfo[NSLocalizedDescriptionKey]);
}

- (void)testIMStatusBridgedErrorRoundTripsThroughErrorHolder
{
    // The IM-status branch (errorCode.IsIMStatus()) attaches an MTRErrorHolder so the
    // NSError (Matter) category accessors (mtr_underlyingMatterErrorSourceFile/Line) work for
    // bridged IM-status errors. (Regression guard for copilot's catch — without the holder, the
    // category accessors silently return nil/0 on this code path.)
    const uint32_t kImStatusFailure = (5u << 8) | 0x01u; // SdkPart::kIMGlobalStatus << 8 | Status::Failure

    NSError * bridged = [MTRError errorForCHIPIntegerCode:kImStatusFailure];
    XCTAssertEqualObjects(bridged.domain, MTRInteractionErrorDomain);

    // Round-trip for IM-status errors works via StatusIB::ToChipError() reconstruction:
    // errorToCHIPErrorCode: short-circuits on MTRInteractionErrorDomain BEFORE consulting any
    // MTRErrorHolder. The holder is attached so that mtr_underlyingMatterErrorSourceFile/Line
    // accessors return source-location info; it does NOT participate in the integer round-trip.
    XCTAssertEqual([MTRError errorToCHIPIntegerCode:bridged], kImStatusFailure);

#if CHIP_CONFIG_ERROR_SOURCE
    // Pin the actual benefit of the holder-attach on the IM path: the category accessors must
    // return non-empty source-location info. If a future change removes the objc_setAssociatedObject
    // call on the IM path, this assertion will fail even though the integer round-trip above
    // would silently keep working.
    XCTAssertNotNil(bridged.mtr_underlyingMatterErrorSourceFile);
    XCTAssertGreaterThan(bridged.mtr_underlyingMatterErrorSourceLine, (NSUInteger) 0);
#endif
}

- (void)testAttestationUserInfoKeysPopulatedOnIntegrityCheckBridge
{
    // CHIP_ERROR_INTEGRITY_CHECK_FAILED = CHIP_CORE_ERROR(0x13) = 0x00000013.
    const uint32_t kIntegrityCheckFailed = 0x00000013u;
    NSDictionary * attestationInfo = @{
        MTRAttestationVerificationResultKey : @(502),
        MTRDeviceBasicInformationVendorIDKey : @(0xFFF1u),
        MTRDeviceBasicInformationProductIDKey : @(0x8000u),
    };

    NSError * bridged = [MTRError errorForCHIPIntegerCode:kIntegrityCheckFailed
                                       additionalUserInfo:attestationInfo];

    XCTAssertEqualObjects(bridged.domain, MTRErrorDomain);
    XCTAssertEqual((NSInteger) bridged.code, MTRErrorCodeIntegrityCheckFailed);

    id underlying = bridged.userInfo[MTRUnderlyingErrorCodeKey];
    XCTAssertTrue([underlying isKindOfClass:[NSNumber class]]);
    XCTAssertEqual([(NSNumber *) underlying unsignedIntValue], kIntegrityCheckFailed);

    XCTAssertEqualObjects(bridged.userInfo[MTRAttestationVerificationResultKey], @(502));
    XCTAssertEqualObjects(bridged.userInfo[MTRDeviceBasicInformationVendorIDKey], @(0xFFF1u));
    XCTAssertEqualObjects(bridged.userInfo[MTRDeviceBasicInformationProductIDKey], @(0x8000u));

    XCTAssertEqual([MTRError errorToCHIPIntegerCode:bridged], kIntegrityCheckFailed);
}

- (void)testKOSRangeChipErrorRoundTripsCoreBluetoothCode
{
    // CHIP_ERROR(Range::kOS, 3) encodes as (Range::kOS << 24) | 3 = 0x01000003.
    // The kOS range has no specific MTRErrorCode mapping; falls back to MTRErrorCodeGeneralError.
    // MTRUnderlyingErrorCodeKey must carry the full integer; round-trip via MTRErrorHolder.
    const uint32_t kCBOperationCancelled = 0x01000003u;

    NSError * bridged = [MTRError errorForCHIPIntegerCode:kCBOperationCancelled];
    XCTAssertEqualObjects(bridged.domain, MTRErrorDomain);
    XCTAssertEqual((NSInteger) bridged.code, MTRErrorCodeGeneralError);

    id underlying = bridged.userInfo[MTRUnderlyingErrorCodeKey];
    XCTAssertTrue([underlying isKindOfClass:[NSNumber class]]);
    XCTAssertEqual([(NSNumber *) underlying unsignedIntValue], kCBOperationCancelled);

    XCTAssertEqual([MTRError errorToCHIPIntegerCode:bridged], kCBOperationCancelled);
}

- (void)testKOSRangeDistinguishesCoreBluetoothErrorCodes
{
    // Two distinct kOS-range error codes must produce two distinct underlying-code values
    // (regression guard against re-collapse to CHIP_ERROR_INTERNAL).
    const uint32_t kCBConnectionFailed = 0x01000002u;
    const uint32_t kCBOperationCancelled = 0x01000003u;

    NSError * err1 = [MTRError errorForCHIPIntegerCode:kCBConnectionFailed];
    NSError * err2 = [MTRError errorForCHIPIntegerCode:kCBOperationCancelled];

    uint32_t code1 = [(NSNumber *) err1.userInfo[MTRUnderlyingErrorCodeKey] unsignedIntValue];
    uint32_t code2 = [(NSNumber *) err2.userInfo[MTRUnderlyingErrorCodeKey] unsignedIntValue];

    XCTAssertNotEqual(code1, code2);
    XCTAssertEqual(code1, kCBConnectionFailed);
    XCTAssertEqual(code2, kCBOperationCancelled);
}

- (void)testAllChipRangesProduceDistinctUnderlyingCodeKeys
{
    // Pick a representative non-zero error code from each CHIP_ERROR Range/SdkPart family that
    // the bridge needs to disambiguate. None of these should collide on the
    // MTRUnderlyingErrorCodeKey integer — every range/part must encode into a different bit
    // pattern so callers can recover the original CHIP_ERROR after bridging.
    //
    // Encoding (see src/lib/core/CHIPError.h):
    //   Range::kSDK  = 0x0  (kSDK errors:  (SdkPart << 8) | code)
    //   Range::kOS   = 0x1  → 0x01000000 base
    //   Range::kPlatform = 0x5 → 0x05000000 base
    // SdkPart inside kSDK: kCore=0, kInet=1, kASN1=3, kBLE=4.
    NSDictionary<NSString *, NSNumber *> * samples = @{
        @"Range::kSDK / SdkPart::kCore" : @(0x0000000Bu), // CHIP_ERROR_NO_MEMORY
        @"Range::kSDK / SdkPart::kInet" : @(0x00000101u), // arbitrary kInet code
        @"Range::kSDK / SdkPart::kASN1" : @(0x00000301u), // arbitrary kASN1 code
        @"Range::kSDK / SdkPart::kBLE" : @(0x00000401u), // arbitrary kBLE code
        @"Range::kOS" : @(0x01000003u), // CBErrorOperationCancelled-shaped
        @"Range::kPlatform" : @(0x05000007u), // arbitrary kPlatform code
    };

    NSMutableDictionary<NSNumber *, NSString *> * seen = [NSMutableDictionary dictionary];
    for (NSString * label in samples) {
        uint32_t chipCode = [samples[label] unsignedIntValue];
        NSError * bridged = [MTRError errorForCHIPIntegerCode:chipCode];
        XCTAssertEqualObjects(bridged.domain, MTRErrorDomain, @"%@", label);

        id underlying = bridged.userInfo[MTRUnderlyingErrorCodeKey];
        XCTAssertTrue([underlying isKindOfClass:[NSNumber class]],
            @"MTRUnderlyingErrorCodeKey missing for %@ (chip 0x%08X)", label, chipCode);
        NSNumber * key = (NSNumber *) underlying;
        XCTAssertEqual([key unsignedIntValue], chipCode, @"%@ key value mismatch", label);

        NSString * prior = seen[key];
        XCTAssertNil(prior, @"Underlying-code collision: %@ collides with %@ (key=0x%08X)",
            label, prior, [key unsignedIntValue]);
        seen[key] = label;

        // Round-trip: errorToCHIPIntegerCode: must recover the exact integer for every range.
        XCTAssertEqual([MTRError errorToCHIPIntegerCode:bridged], chipCode,
            @"Round-trip failed for %@ (chip 0x%08X)", label, chipCode);
    }

    XCTAssertEqual(seen.count, samples.count, @"All sampled ranges must produce distinct keys");
}

- (void)testAdditionalUserInfoFrameworkReservedKeysWinOnNonIMPath
{
    // Non-IM-status path: framework-reserved keys (NSLocalizedDescriptionKey,
    // MTRUnderlyingErrorCodeKey) CANNOT be overridden by additionalUserInfo. The bridge owns
    // MTRUnderlyingErrorCodeKey because errorToCHIPErrorCode: relies on it to recover the
    // original CHIP_ERROR integer when the MTRErrorHolder associated object is absent
    // (post-XPC, post-NSCoding, post-custom-NSCopying). Allowing a caller-supplied value to
    // overwrite it would silently corrupt the integer round-trip and lets additionalUserInfo
    // poison upstream code that branches on the integer. Non-reserved caller keys still pass
    // through.

    NSString * const callerDescription = @"caller desc";
    NSNumber * const callerUnderlyingCode = @(0xDEADBEEFu);
    const uint32_t kChipErrorNoMemory = 0x0000000Bu;

    NSDictionary * coreCollision = @{
        NSLocalizedDescriptionKey : callerDescription,
        MTRUnderlyingErrorCodeKey : callerUnderlyingCode,
        @"customDebugInfo" : @"foo",
    };
    NSError * coreBridged = [MTRError errorForCHIPIntegerCode:kChipErrorNoMemory
                                           additionalUserInfo:coreCollision];
    XCTAssertEqualObjects(coreBridged.domain, MTRErrorDomain);

    // Framework wins: NSLocalizedDescriptionKey must reflect the bridge's localized description,
    // not the caller-supplied string.
    XCTAssertNotEqualObjects(coreBridged.userInfo[NSLocalizedDescriptionKey], callerDescription,
        @"Bridge must override caller-supplied NSLocalizedDescriptionKey on the non-IM-status path");
    XCTAssertNotNil(coreBridged.userInfo[NSLocalizedDescriptionKey]);

    // Framework wins: MTRUnderlyingErrorCodeKey must equal the bridged CHIP_ERROR integer so
    // errorToCHIPErrorCode: round-trip is not corrupted by additionalUserInfo.
    XCTAssertEqualObjects(coreBridged.userInfo[MTRUnderlyingErrorCodeKey], @(kChipErrorNoMemory),
        @"Bridge must override caller-supplied MTRUnderlyingErrorCodeKey so errorToCHIPErrorCode: round-trip is not corrupted");

    // Non-reserved caller keys must still pass through.
    XCTAssertEqualObjects(coreBridged.userInfo[@"customDebugInfo"], @"foo");
}

- (void)testRoundTripUnderlyingErrorCodeNotCorruptibleByCaller
{
    // Regression: even after additionalUserInfo attempts to poison MTRUnderlyingErrorCodeKey,
    // a caller that recovers CHIP_ERROR via the userInfo integer (i.e. without relying on the
    // MTRErrorHolder associated object — simulating post-XPC / post-NSCoding) must still see
    // the bridge's authoritative integer.

    const uint32_t kChipErrorNoMemory = 0x0000000Bu;
    NSDictionary * poison = @{
        MTRUnderlyingErrorCodeKey : @(0xDEADBEEFu),
        NSLocalizedDescriptionKey : @"poisoned description",
    };
    NSError * bridged = [MTRError errorForCHIPIntegerCode:kChipErrorNoMemory
                                       additionalUserInfo:poison];
    XCTAssertNotNil(bridged);
    XCTAssertEqualObjects(bridged.domain, MTRErrorDomain);

    // Read the integer the way a downstream consumer would after the MTRErrorHolder is gone:
    // straight off userInfo[MTRUnderlyingErrorCodeKey].
    NSNumber * underlying = bridged.userInfo[MTRUnderlyingErrorCodeKey];
    XCTAssertNotNil(underlying);
    XCTAssertEqualObjects(underlying, @(kChipErrorNoMemory),
        @"caller-supplied additionalUserInfo must not corrupt the bridged underlying error integer");

    // And the full errorToCHIPIntegerCode: round-trip (which prefers the holder, but must
    // also be robust against poisoned userInfo) must agree.
    XCTAssertEqual([MTRError errorToCHIPIntegerCode:bridged], kChipErrorNoMemory,
        @"errorToCHIPIntegerCode: must recover the bridged integer regardless of caller additionalUserInfo");
}

- (void)testAdditionalUserInfoBridgeWinsForReservedKeysOnIMStatusPath
{
    // IM-status path: framework-reserved keys (@"clusterStatus", NSLocalizedDescriptionKey)
    // CANNOT be overridden by additionalUserInfo. errorToCHIPErrorCode: re-encodes
    // StatusIB.mClusterStatus from userInfo[@"clusterStatus"] on the IM-status fast path, so a
    // caller-supplied @"clusterStatus" would silently corrupt the integer round-trip.
    // Non-reserved caller keys still pass through.

    NSString * const callerDescription = @"caller-supplied description (must be overridden on IM path)";
    NSNumber * const callerClusterStatus = @(0x42u);

    // CHIP_ERROR encoding for IM-status with cluster-specific status:
    // SdkPart::kIMClusterStatus is 6. The lower byte is the cluster-specific status that
    // StatusIB encodes as mClusterStatus and the bridge surfaces in userInfo[@"clusterStatus"].
    const uint32_t kImClusterStatus = (6u << 8) | 0x05u; // SdkPart::kIMClusterStatus | clusterStatus=5

    NSDictionary * imCollision = @{
        NSLocalizedDescriptionKey : callerDescription,
        @"clusterStatus" : callerClusterStatus,
        @"customCallerKey" : @"customCallerValue",
    };
    NSError * imBridged = [MTRError errorForCHIPIntegerCode:kImClusterStatus
                                         additionalUserInfo:imCollision];
    XCTAssertEqualObjects(imBridged.domain, MTRInteractionErrorDomain);

    // Bridge wins for the framework-reserved description key on the IM path.
    XCTAssertNotEqualObjects(imBridged.userInfo[NSLocalizedDescriptionKey], callerDescription,
        @"Bridge must override caller-supplied NSLocalizedDescriptionKey on the IM-status path");
    XCTAssertNotNil(imBridged.userInfo[NSLocalizedDescriptionKey]);

    // Bridge wins for @"clusterStatus" so the round-trip integer is preserved.
    XCTAssertNotEqualObjects(imBridged.userInfo[@"clusterStatus"], callerClusterStatus,
        @"Bridge must override caller-supplied @\"clusterStatus\" so errorToCHIPErrorCode: round-trip is not corrupted");
    XCTAssertEqual([MTRError errorToCHIPIntegerCode:imBridged], kImClusterStatus,
        @"Round-trip via StatusIB re-encoding must recover the original CHIP_ERROR; caller-supplied @\"clusterStatus\" must not corrupt it");

    // Non-reserved caller keys must still pass through.
    XCTAssertEqualObjects(imBridged.userInfo[@"customCallerKey"], @"customCallerValue");
}

- (void)testNoErrorWithAdditionalUserInfoStillReturnsNil
{
    // CHIP_NO_ERROR (integer 0) must continue to return nil even when callers supply
    // additionalUserInfo. The bridge's CHIP_NO_ERROR early-return must not be bypassed by
    // a non-nil additionalUserInfo argument — passing context for a possibly-failing
    // operation that ultimately succeeds should NOT manufacture a spurious NSError.
    NSDictionary * extras = @{
        MTRAttestationVerificationResultKey : @(0),
        MTRDeviceBasicInformationVendorIDKey : @(0xFFF1u),
        MTRDeviceBasicInformationProductIDKey : @(0x8000u),
        @"customKey" : @"shouldNotAppearAnywhere",
    };
    NSError * shouldBeNil = [MTRError errorForCHIPIntegerCode:0 /* CHIP_NO_ERROR */
                                           additionalUserInfo:extras];
    XCTAssertNil(shouldBeNil,
        @"errorForCHIPIntegerCode:additionalUserInfo: must return nil for CHIP_NO_ERROR regardless of extras");

    // Empty (non-nil) additionalUserInfo dictionary on a real error: must not crash and
    // must not produce stray empty userInfo keys.
    NSError * emptyExtras = [MTRError errorForCHIPIntegerCode:0x0000000Bu /* CHIP_ERROR_NO_MEMORY */
                                           additionalUserInfo:@{}];
    XCTAssertNotNil(emptyExtras);
    XCTAssertEqualObjects(emptyExtras.domain, MTRErrorDomain);
    XCTAssertNotNil(emptyExtras.userInfo[MTRUnderlyingErrorCodeKey],
        @"Bridge must still populate MTRUnderlyingErrorCodeKey even when additionalUserInfo is an empty dict");
}

- (void)testErrorToChipIntegerCodeFallsBackToLegacyUserInfoKeyWithoutHolder
{
    // External code (and historical framework code) may construct an NSError manually with
    // domain=MTRErrorDomain, code=MTRErrorCodeGeneralError, and userInfo={ @"errorCode": N }
    // — that is the documented public contract for MTRErrorCodeGeneralError. Such an NSError
    // does NOT have an MTRErrorHolder associated. errorToCHIPIntegerCode: must still recover
    // the underlying integer from the userInfo string key in that case (regression guard for
    // the diff's switch from the literal @"errorCode" to the MTRUnderlyingErrorCodeKey constant).
    const uint32_t kArbitraryUnderlying = 0x12345678u;
    NSError * manuallyBuilt = [NSError errorWithDomain:MTRErrorDomain
                                                  code:MTRErrorCodeGeneralError
                                              userInfo:@{ @"errorCode" : @(kArbitraryUnderlying) }];
    XCTAssertEqual([MTRError errorToCHIPIntegerCode:manuallyBuilt], kArbitraryUnderlying,
        @"errorToCHIPIntegerCode: must read the legacy @\"errorCode\" userInfo key when no MTRErrorHolder is attached");

    // Same NSError built via the public MTRUnderlyingErrorCodeKey constant must behave
    // identically (the constant is documented to resolve to @"errorCode").
    NSError * viaConstant = [NSError errorWithDomain:MTRErrorDomain
                                                code:MTRErrorCodeGeneralError
                                            userInfo:@{ MTRUnderlyingErrorCodeKey : @(kArbitraryUnderlying) }];
    XCTAssertEqual([MTRError errorToCHIPIntegerCode:viaConstant], kArbitraryUnderlying,
        @"errorToCHIPIntegerCode: must work with the MTRUnderlyingErrorCodeKey constant identically to the legacy string");

    // GeneralError with no underlying-code userInfo should not crash and should fall through
    // to the bridge's default for an un-mappable GeneralError.
    NSError * noUnderlying = [NSError errorWithDomain:MTRErrorDomain
                                                 code:MTRErrorCodeGeneralError
                                             userInfo:nil];
    // We don't pin the exact CHIP_ERROR returned here (it's an internal fallback); just
    // require that the call returns a non-zero (i.e., error) integer and does not crash.
    XCTAssertNotEqual([MTRError errorToCHIPIntegerCode:noUnderlying], 0u,
        @"errorToCHIPIntegerCode: must produce a non-success integer for a GeneralError with no underlying code");
}

- (void)testNewDNSSDErrorCodesBridgeAndRoundTripThroughGeneralError
{
    // PR-introduced CHIP_ERROR codes:
    //   CHIP_ERROR_DNS_SD_NXDOMAIN             = CHIP_CORE_ERROR(0xbe) = 0x000000be
    //   CHIP_ERROR_DNS_SD_SERVICE_NOT_RUNNING  = CHIP_CORE_ERROR(0xc7) = 0x000000c7
    // Neither has a dedicated MTRErrorCode mapping yet — the bridge must fall through to
    // MTRErrorCodeGeneralError, populate MTRUnderlyingErrorCodeKey with the exact integer,
    // and round-trip cleanly. This guards against a future "specific" mapping silently
    // changing the public surface (callers reading userInfo[@"errorCode"]) without an
    // accompanying MTRErrorCode addition.
    const uint32_t kDnssdNxdomain = 0x000000beu;
    const uint32_t kDnssdServiceNotRunning = 0x000000c7u;

    NSError * nxdomain = [MTRError errorForCHIPIntegerCode:kDnssdNxdomain];
    XCTAssertEqualObjects(nxdomain.domain, MTRErrorDomain);
    XCTAssertEqual((NSInteger) nxdomain.code, MTRErrorCodeGeneralError,
        @"CHIP_ERROR_DNS_SD_NXDOMAIN currently has no specific MTRErrorCode mapping; falls back to GeneralError");
    XCTAssertEqualObjects(nxdomain.userInfo[MTRUnderlyingErrorCodeKey], @(kDnssdNxdomain));
    XCTAssertEqual([MTRError errorToCHIPIntegerCode:nxdomain], kDnssdNxdomain);

    NSError * serviceNotRunning = [MTRError errorForCHIPIntegerCode:kDnssdServiceNotRunning];
    XCTAssertEqualObjects(serviceNotRunning.domain, MTRErrorDomain);
    XCTAssertEqual((NSInteger) serviceNotRunning.code, MTRErrorCodeGeneralError);
    XCTAssertEqualObjects(serviceNotRunning.userInfo[MTRUnderlyingErrorCodeKey], @(kDnssdServiceNotRunning));
    XCTAssertEqual([MTRError errorToCHIPIntegerCode:serviceNotRunning], kDnssdServiceNotRunning);

    // The two new codes must not collide with each other or with the pre-existing
    // CHIP_ERROR_DNS_SD_UNAUTHORIZED — all three are distinct DNS-SD failure modes that
    // triage scripts need to disambiguate.
    const uint32_t kDnssdUnauthorized = 0x0000005bu; // CHIP_ERROR_DNS_SD_UNAUTHORIZED = 0x5b
    NSError * unauthorized = [MTRError errorForCHIPIntegerCode:kDnssdUnauthorized];
    uint32_t a = [(NSNumber *) nxdomain.userInfo[MTRUnderlyingErrorCodeKey] unsignedIntValue];
    uint32_t b = [(NSNumber *) serviceNotRunning.userInfo[MTRUnderlyingErrorCodeKey] unsignedIntValue];
    uint32_t c = [(NSNumber *) unauthorized.userInfo[MTRUnderlyingErrorCodeKey] unsignedIntValue];
    XCTAssertNotEqual(a, b);
    XCTAssertNotEqual(a, c);
    XCTAssertNotEqual(b, c);
}

- (void)testKOSRangeBoundaryValuesRoundTripCleanly
{
    // The kOS range only has 24 bits of value space (see ChipError::MaskValue / the
    // WrapCBErrorCodeAsKOS guard added by this PR for CoreBluetooth NSError.code values).
    // Pin both ends of the representable interval — the bridge must preserve the full
    // 24-bit value and not silently truncate or alias to CHIP_ERROR_INTERNAL.
    //
    // Encoded form: (Range::kOS << 24) | value, with kOS = 0x1.
    const uint32_t kKOSMin = 0x01000000u; // kOS, value=0
    const uint32_t kKOSMax = 0x01FFFFFFu; // kOS, value=0xFFFFFF (24-bit max)
    const uint32_t kKOSMid = 0x01ABCDEFu; // arbitrary mid-range value

    const uint32_t kSamples[] = { kKOSMin, kKOSMax, kKOSMid };
    for (size_t i = 0; i < sizeof(kSamples) / sizeof(kSamples[0]); ++i) {
        uint32_t code = kSamples[i];
        NSError * bridged = [MTRError errorForCHIPIntegerCode:code];
        XCTAssertEqualObjects(bridged.domain, MTRErrorDomain, @"kOS code 0x%08X", code);
        // Note: kKOSMin (0x01000000) is technically CHIP_NO_ERROR-shaped (value bits zero) but
        // the upper byte sets Range::kOS, so AsInteger() != 0 and the bridge must NOT treat it
        // as success.
        XCTAssertEqual((NSInteger) bridged.code, MTRErrorCodeGeneralError, @"kOS code 0x%08X", code);
        id underlying = bridged.userInfo[MTRUnderlyingErrorCodeKey];
        XCTAssertTrue([underlying isKindOfClass:[NSNumber class]], @"kOS code 0x%08X", code);
        XCTAssertEqual([(NSNumber *) underlying unsignedIntValue], code,
            @"kOS code 0x%08X must round-trip exactly through MTRUnderlyingErrorCodeKey", code);
        XCTAssertEqual([MTRError errorToCHIPIntegerCode:bridged], code,
            @"kOS code 0x%08X must round-trip exactly through errorToCHIPIntegerCode:", code);
    }

    // Distinct boundary values must produce distinct underlying-code keys (no aliasing).
    NSError * minErr = [MTRError errorForCHIPIntegerCode:kKOSMin];
    NSError * maxErr = [MTRError errorForCHIPIntegerCode:kKOSMax];
    XCTAssertNotEqualObjects(minErr.userInfo[MTRUnderlyingErrorCodeKey],
        maxErr.userInfo[MTRUnderlyingErrorCodeKey]);
}

- (void)testAttestationUserInfoSurvivesOnNonIntegrityCheckErrorCode
{
    // The attestation-context userInfo keys are populated at the point of failure (the
    // attestation delegate bridge always passes them as additionalUserInfo). The PR's
    // contract is that the keys flow through the bridge regardless of which CHIP_ERROR
    // the attestation result happens to map to — not just CHIP_ERROR_INTEGRITY_CHECK_FAILED.
    // Pin that the merge plumbing is value-agnostic so a future change to the attestation
    // delegate (e.g., reporting CHIP_ERROR_INVALID_ARGUMENT for malformed CD blobs) still
    // surfaces VID/PID/result to callers.
    NSDictionary * attestationInfo = @{
        MTRAttestationVerificationResultKey : @(101), // kPaaNotFound
        MTRDeviceBasicInformationVendorIDKey : @(0xFFF1u),
        MTRDeviceBasicInformationProductIDKey : @(0x8000u),
    };

    // CHIP_ERROR_INVALID_ARGUMENT lands on a SPECIFIC MTRErrorCode (not GeneralError); verify
    // additionalUserInfo merges cleanly there.
    const uint32_t kInvalidArgument = 0x0000002Fu;
    NSError * specific = [MTRError errorForCHIPIntegerCode:kInvalidArgument
                                        additionalUserInfo:attestationInfo];
    XCTAssertEqualObjects(specific.domain, MTRErrorDomain);
    XCTAssertEqual((NSInteger) specific.code, MTRErrorCodeInvalidArgument);
    XCTAssertEqualObjects(specific.userInfo[MTRAttestationVerificationResultKey], @(101));
    XCTAssertEqualObjects(specific.userInfo[MTRDeviceBasicInformationVendorIDKey], @(0xFFF1u));
    XCTAssertEqualObjects(specific.userInfo[MTRDeviceBasicInformationProductIDKey], @(0x8000u));
    // Bridge-set keys (description, underlying code) survive alongside the attestation context.
    XCTAssertNotNil(specific.userInfo[NSLocalizedDescriptionKey]);
    XCTAssertEqualObjects(specific.userInfo[MTRUnderlyingErrorCodeKey], @(kInvalidArgument));
    XCTAssertEqual([MTRError errorToCHIPIntegerCode:specific], kInvalidArgument);

    // CHIP_ERROR with no specific mapping (CHIP_ERROR_NO_MEMORY) — falls through to
    // GeneralError; same merge behavior.
    const uint32_t kNoMemory = 0x0000000Bu;
    NSError * general = [MTRError errorForCHIPIntegerCode:kNoMemory
                                       additionalUserInfo:attestationInfo];
    XCTAssertEqualObjects(general.domain, MTRErrorDomain);
    XCTAssertEqual((NSInteger) general.code, MTRErrorCodeGeneralError);
    XCTAssertEqualObjects(general.userInfo[MTRAttestationVerificationResultKey], @(101));
    XCTAssertEqualObjects(general.userInfo[MTRDeviceBasicInformationVendorIDKey], @(0xFFF1u));
    XCTAssertEqualObjects(general.userInfo[MTRDeviceBasicInformationProductIDKey], @(0x8000u));
    XCTAssertEqual([MTRError errorToCHIPIntegerCode:general], kNoMemory);
}

- (void)testCategoryAccessorsReturnEmptyForArbitraryNonBridgedNSError
{
    // The NSError(Matter) category docstring guarantees the accessors return nil/0 for
    // NSErrors that were not produced by the MTRError bridge (e.g., externally constructed
    // NSErrors, NSErrors that lost their associated object via NSCoding/copy/XPC).
    // Pin that contract — code reading these accessors on an arbitrary NSError
    // must see nil/0 rather than crashing or returning a bogus value.

    // 1) Wholly unrelated domain.
    NSError * cocoaError = [NSError errorWithDomain:NSCocoaErrorDomain code:42 userInfo:nil];
    XCTAssertNil(cocoaError.mtr_underlyingMatterErrorSourceFile);
    XCTAssertEqual(cocoaError.mtr_underlyingMatterErrorSourceLine, (NSUInteger) 0);

    // 2) MTRErrorDomain but constructed manually (no MTRErrorHolder attached).
    NSError * manualMtrError = [NSError errorWithDomain:MTRErrorDomain
                                                   code:MTRErrorCodeInvalidArgument
                                               userInfo:nil];
    XCTAssertNil(manualMtrError.mtr_underlyingMatterErrorSourceFile);
    XCTAssertEqual(manualMtrError.mtr_underlyingMatterErrorSourceLine, (NSUInteger) 0);

    // 3) MTRInteractionErrorDomain but constructed manually.
    NSError * manualInteractionError = [NSError errorWithDomain:MTRInteractionErrorDomain
                                                           code:MTRInteractionErrorCodeFailure
                                                       userInfo:nil];
    XCTAssertNil(manualInteractionError.mtr_underlyingMatterErrorSourceFile);
    XCTAssertEqual(manualInteractionError.mtr_underlyingMatterErrorSourceLine, (NSUInteger) 0);
}

@end
