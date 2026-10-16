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

// NOTE: This is a .mm file, so that it can include MTRError_Internal.h

#import "MTRError_Test.h"

@interface MTRErrorMappingTests : XCTestCase
@end

static const uint32_t kNoMemory = 0x0B;
static const uint32_t kIntegrityCheckFailed = 0x13;
static const uint32_t kIMStatusFailure = 0x501;

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

- (void)testCASEErrorsRoundTripWithoutAssociatedHolder
{
    const uint32_t caseErrors[] = { 0xAA, 0xB1, 0xB2, 0xB7, 0xB8, 0xBE };
    for (size_t i = 0; i < sizeof(caseErrors) / sizeof(caseErrors[0]); i++) {
        uint32_t chipError = caseErrors[i];
        NSError * error = [MTRError errorForCHIPIntegerCode:chipError];
        XCTAssertEqualObjects(error.domain, MTRErrorDomain);
        XCTAssertEqual(error.code, MTRErrorCodeGeneralError);

        NSError * stripped = [NSError errorWithDomain:error.domain code:error.code userInfo:error.userInfo];
        XCTAssertEqual([MTRError errorToCHIPIntegerCode:stripped], chipError, "Testing CHIP error 0x%x", chipError);
    }
}

- (void)testUnderlyingErrorCode
{
    XCTAssertEqualObjects(MTRUnderlyingErrorCodeKey, @"errorCode");

    NSError * generalError = [MTRError errorForCHIPIntegerCode:kNoMemory];
    XCTAssertEqual(generalError.code, MTRErrorCodeGeneralError);
    XCTAssertEqualObjects(generalError.userInfo[MTRUnderlyingErrorCodeKey], @(kNoMemory));

    NSError * mappedError = [MTRError errorForCHIPIntegerCode:kIntegrityCheckFailed];
    XCTAssertEqual(mappedError.code, MTRErrorCodeIntegrityCheckFailed);
    XCTAssertEqualObjects(mappedError.userInfo[MTRUnderlyingErrorCodeKey], @(kIntegrityCheckFailed));

    NSError * imError = [MTRError errorForCHIPIntegerCode:kIMStatusFailure];
    XCTAssertEqualObjects(imError.domain, MTRInteractionErrorDomain);
    XCTAssertEqual(imError.code, MTRInteractionErrorCodeFailure);
    XCTAssertEqualObjects(imError.userInfo[MTRUnderlyingErrorCodeKey], @(kIMStatusFailure));
    XCTAssertEqual([MTRError errorToCHIPIntegerCode:imError], kIMStatusFailure);
}

- (void)testAdditionalUserInfo
{
    NSDictionary * extraUserInfo = @{ @"someKey" : @"someValue", MTRUnderlyingErrorCodeKey : @(12345) };

    NSError * error = [MTRError errorForCHIPIntegerCode:kIntegrityCheckFailed additionalUserInfo:extraUserInfo];
    XCTAssertEqualObjects(error.domain, MTRErrorDomain);
    XCTAssertEqual(error.code, MTRErrorCodeIntegrityCheckFailed);
    XCTAssertEqualObjects(error.userInfo[@"someKey"], @"someValue");
    XCTAssertEqualObjects(error.userInfo[MTRUnderlyingErrorCodeKey], @(kIntegrityCheckFailed));
    XCTAssertNotNil(error.userInfo[NSLocalizedDescriptionKey]);
    XCTAssertEqual([MTRError errorToCHIPIntegerCode:error], kIntegrityCheckFailed);

    NSError * imError = [MTRError errorForCHIPIntegerCode:kIMStatusFailure additionalUserInfo:extraUserInfo];
    XCTAssertEqualObjects(imError.domain, MTRInteractionErrorDomain);
    XCTAssertEqual(imError.code, MTRInteractionErrorCodeFailure);
    XCTAssertEqualObjects(imError.userInfo[@"someKey"], @"someValue");
    XCTAssertEqualObjects(imError.userInfo[MTRUnderlyingErrorCodeKey], @(kIMStatusFailure));
    XCTAssertNotNil(imError.userInfo[NSLocalizedDescriptionKey]);

    XCTAssertNil([MTRError errorForCHIPIntegerCode:0 additionalUserInfo:extraUserInfo]);
}

@end
