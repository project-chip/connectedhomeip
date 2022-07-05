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

#import <Matter/Matter.h>

// system dependencies
#import <XCTest/XCTest.h>

#import "MTRTestKeys.h"

@interface MatterCertificateTests : XCTestCase

@end

@implementation MatterCertificateTests

- (void)testGenerateRootCert
{
    __auto_type * testKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    __auto_type * rootCert = [MTRCertificates generateRootCertificate:testKeys issuerId:nil fabricId:nil error:nil];
    XCTAssertNotNil(rootCert);
}

- (void)testGenerateIntermediateCert
{
    __auto_type * rootKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * rootCert = [MTRCertificates generateRootCertificate:rootKeys issuerId:nil fabricId:nil error:nil];
    XCTAssertNotNil(rootCert);

    __auto_type * intermediateKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(intermediateKeys);

    __auto_type * intermediateCert = [MTRCertificates generateIntermediateCertificate:rootKeys
                                                                      rootCertificate:rootCert
                                                                intermediatePublicKey:intermediateKeys.publicKey
                                                                             issuerId:nil
                                                                             fabricId:nil
                                                                                error:nil];
    XCTAssertNotNil(intermediateCert);
}

- (void)testGenerateOperationalCertNoIntermediate
{
    __auto_type * rootKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * rootCert = [MTRCertificates generateRootCertificate:rootKeys issuerId:nil fabricId:nil error:nil];
    XCTAssertNotNil(rootCert);

    __auto_type * operationalKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(operationalKeys);

    __auto_type * cats = [[NSMutableArray alloc] initWithCapacity:3];
    [cats addObject:@1];
    [cats addObject:@2];
    [cats addObject:@3];

    __auto_type * operationalCert = [MTRCertificates generateOperationalCertificate:rootKeys
                                                                 signingCertificate:rootCert
                                                               operationalPublicKey:operationalKeys.publicKey
                                                                           fabricId:@1
                                                                             nodeId:@1
                                                              caseAuthenticatedTags:cats
                                                                              error:nil];
    XCTAssertNotNil(operationalCert);
}

- (void)testGenerateOperationalCertWithIntermediate
{
    __auto_type * rootKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * rootCert = [MTRCertificates generateRootCertificate:rootKeys issuerId:nil fabricId:nil error:nil];
    XCTAssertNotNil(rootCert);

    __auto_type * intermediateKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(intermediateKeys);

    __auto_type * intermediateCert = [MTRCertificates generateIntermediateCertificate:rootKeys
                                                                      rootCertificate:rootCert
                                                                intermediatePublicKey:intermediateKeys.publicKey
                                                                             issuerId:nil
                                                                             fabricId:nil
                                                                                error:nil];
    XCTAssertNotNil(intermediateCert);

    __auto_type * operationalKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(operationalKeys);

    __auto_type * operationalCert = [MTRCertificates generateOperationalCertificate:intermediateKeys
                                                                 signingCertificate:intermediateCert
                                                               operationalPublicKey:operationalKeys.publicKey
                                                                           fabricId:@1
                                                                             nodeId:@1
                                                              caseAuthenticatedTags:nil
                                                                              error:nil];
    XCTAssertNotNil(operationalCert);
}

- (void)testGenerateOperationalCertErrorCases
{
    __auto_type * rootKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * rootCert = [MTRCertificates generateRootCertificate:rootKeys issuerId:nil fabricId:nil error:nil];
    XCTAssertNotNil(rootCert);

    __auto_type * operationalKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(operationalKeys);

    __auto_type * cats = [[NSMutableArray alloc] initWithCapacity:4];
    [cats addObject:@1];
    [cats addObject:@2];
    [cats addObject:@3];
    [cats addObject:@4];

    // Check basic case works
    __auto_type * operationalCert = [MTRCertificates generateOperationalCertificate:rootKeys
                                                                 signingCertificate:rootCert
                                                               operationalPublicKey:operationalKeys.publicKey
                                                                           fabricId:@1
                                                                             nodeId:@1
                                                              caseAuthenticatedTags:nil
                                                                              error:nil];
    XCTAssertNotNil(operationalCert);

    // CATs too long
    operationalCert = [MTRCertificates generateOperationalCertificate:rootKeys
                                                   signingCertificate:rootCert
                                                 operationalPublicKey:operationalKeys.publicKey
                                                             fabricId:@1
                                                               nodeId:@1
                                                caseAuthenticatedTags:cats
                                                                error:nil];
    XCTAssertNil(operationalCert);

    // Signing key mismatch
    operationalCert = [MTRCertificates generateOperationalCertificate:operationalKeys
                                                   signingCertificate:rootCert
                                                 operationalPublicKey:operationalKeys.publicKey
                                                             fabricId:@1
                                                               nodeId:@1
                                                caseAuthenticatedTags:nil
                                                                error:nil];
    XCTAssertNil(operationalCert);

    // Invalid fabric id
    operationalCert = [MTRCertificates generateOperationalCertificate:rootKeys
                                                   signingCertificate:rootCert
                                                 operationalPublicKey:operationalKeys.publicKey
                                                             fabricId:@0
                                                               nodeId:@1
                                                caseAuthenticatedTags:nil
                                                                error:nil];
    XCTAssertNil(operationalCert);

    // Undefined node id
    operationalCert = [MTRCertificates generateOperationalCertificate:rootKeys
                                                   signingCertificate:rootCert
                                                 operationalPublicKey:operationalKeys.publicKey
                                                             fabricId:@1
                                                               nodeId:@0
                                                caseAuthenticatedTags:nil
                                                                error:nil];
    XCTAssertNil(operationalCert);

    // Non-operational node id
    operationalCert = [MTRCertificates generateOperationalCertificate:rootKeys
                                                   signingCertificate:rootCert
                                                 operationalPublicKey:operationalKeys.publicKey
                                                             fabricId:@1
                                                               nodeId:@(0xFFFFFFFFFFFFFFFFLLU)
                                                caseAuthenticatedTags:nil
                                                                error:nil];
    XCTAssertNil(operationalCert);
}

- (void)testGenerateCSR
{
    __auto_type * testKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    __auto_type * csr = [MTRCertificates generateCertificateSigningRequest:testKeys error:nil];
    XCTAssertNotNil(csr);

    // Wish there was something we could test here about the CSR.
}

@end
