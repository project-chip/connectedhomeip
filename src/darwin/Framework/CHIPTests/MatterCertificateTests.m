/**
 *    Copyright (c) 2022 Project CHIP Authors
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

#import <CHIP/CHIP.h>

// system dependencies
#import <XCTest/XCTest.h>

#import "CHIPTestKeys.h"

@interface MatterCertificateTests : XCTestCase

@end

@implementation MatterCertificateTests

- (void)testGenerateRootCert
{
    __auto_type * testKeys = [[CHIPTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    __auto_type * rootCert = [MTRCertificates generateRootCertificate:testKeys issuerId:nil fabricId:nil error:nil];
    XCTAssertNotNil(rootCert);
}

- (void)testGenerateIntermediateCert
{
    __auto_type * rootKeys = [[CHIPTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * rootCert = [MTRCertificates generateRootCertificate:rootKeys issuerId:nil fabricId:nil error:nil];
    XCTAssertNotNil(rootCert);

    __auto_type * intermediateKeys = [[CHIPTestKeys alloc] init];
    XCTAssertNotNil(intermediateKeys);

    __auto_type * intermediateCert = [MTRCertificates generateIntermediateCertificate:rootKeys
                                                                      rootCertificate:rootCert
                                                                intermediatePublicKey:intermediateKeys.pubkey
                                                                             issuerId:nil
                                                                             fabricId:nil
                                                                                error:nil];
    XCTAssertNotNil(intermediateCert);
}

@end
