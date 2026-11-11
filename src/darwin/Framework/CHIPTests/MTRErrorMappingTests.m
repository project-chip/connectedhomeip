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
        XCTAssertEqualObjects(newError1.domain, error.domain, "Testing error code %d", errorCode);
        XCTAssertEqual(newError1.code, error.code, "Testing error code %d", errorCode);

        __auto_type chipError = [MTRError errorToCHIPIntegerCode:error];
        __auto_type * newError2 = [MTRError errorForCHIPIntegerCode:chipError];
        XCTAssertEqualObjects(newError2.domain, error.domain, "Testing error code %d", errorCode);
        XCTAssertEqual(newError2.code, error.code, "Testing error code %d", errorCode);
    }

    // Check that an unknown value becomes GeneralError.
    __auto_type * error = [MTRError errorWithCode:(MTRErrorCode) (MTRMaxErrorCode + 1)];
    XCTAssertEqualObjects(error.domain, MTRErrorDomain);
    XCTAssertEqual(error.code, MTRMaxErrorCode + 1);

    __auto_type chipError = [MTRError errorToCHIPIntegerCode:error];
    __auto_type * newError = [MTRError errorForCHIPIntegerCode:chipError];
    XCTAssertEqualObjects(newError.domain, MTRErrorDomain);
    XCTAssertEqual(newError.code, MTRErrorCodeGeneralError);
}

- (void)testEqualButDistinctDomainStrings
{
    __auto_type * error = [NSError errorWithDomain:[MTRErrorDomain mutableCopy] code:MTRErrorCodeTimeout userInfo:nil];
    // Deliberately a pointer comparison: the domain must be a different object from MTRErrorDomain.
    XCTAssertTrue(error.domain != MTRErrorDomain);
    __auto_type * constantDomainError = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeTimeout userInfo:nil];
    XCTAssertEqual([MTRError errorToCHIPIntegerCode:error], [MTRError errorToCHIPIntegerCode:constantDomainError]);

    __auto_type * interactionError = [NSError errorWithDomain:[MTRInteractionErrorDomain mutableCopy] code:MTRInteractionErrorCodeBusy userInfo:nil];
    // Deliberately a pointer comparison: the domain must be a different object from MTRInteractionErrorDomain.
    XCTAssertTrue(interactionError.domain != MTRInteractionErrorDomain);
    __auto_type * constantDomainInteractionError = [NSError errorWithDomain:MTRInteractionErrorDomain code:MTRInteractionErrorCodeBusy userInfo:nil];
    XCTAssertEqual([MTRError errorToCHIPIntegerCode:interactionError], [MTRError errorToCHIPIntegerCode:constantDomainInteractionError]);
}

@end
