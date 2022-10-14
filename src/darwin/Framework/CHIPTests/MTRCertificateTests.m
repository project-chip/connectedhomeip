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
    // High bits are identifier, low bits are version.
    [cats addObject:@0x00010001];
    [cats addObject:@0x00020001];
    [cats addObject:@0x0003FFFF];

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

    __auto_type * longCats = [[NSMutableArray alloc] initWithCapacity:4];
    [longCats addObject:@0x00010001];
    [longCats addObject:@0x00020001];
    [longCats addObject:@0x00030001];
    [longCats addObject:@0x00040001];

    __auto_type * catsWithSameIdentifier = [[NSMutableArray alloc] initWithCapacity:3];
    // High bits are identifier, low bits are version.
    [catsWithSameIdentifier addObject:@0x00010001];
    [catsWithSameIdentifier addObject:@0x00020001];
    [catsWithSameIdentifier addObject:@0x00010002];

    __auto_type * catsWithDuplicatedCAT = [[NSMutableArray alloc] initWithCapacity:3];
    // High bits are identifier, low bits are version.
    [catsWithDuplicatedCAT addObject:@0x00010001];
    [catsWithDuplicatedCAT addObject:@0x00020001];
    [catsWithDuplicatedCAT addObject:@0x00010001];

    __auto_type * catsWithInvalidVersion = [[NSMutableArray alloc] initWithCapacity:2];
    // High bits are identifier, low bits are version.
    [catsWithInvalidVersion addObject:@0x00010001];
    [catsWithInvalidVersion addObject:@0x00020000];

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
                                                caseAuthenticatedTags:longCats
                                                                error:nil];
    XCTAssertNil(operationalCert);

    // Multiple CATs with the same identifier but different versions
    operationalCert = [MTRCertificates generateOperationalCertificate:rootKeys
                                                   signingCertificate:rootCert
                                                 operationalPublicKey:operationalKeys.publicKey
                                                             fabricId:@1
                                                               nodeId:@1
                                                caseAuthenticatedTags:catsWithSameIdentifier
                                                                error:nil];
    XCTAssertNil(operationalCert);

    // Multiple CATs with the same identifier and same version
    operationalCert = [MTRCertificates generateOperationalCertificate:rootKeys
                                                   signingCertificate:rootCert
                                                 operationalPublicKey:operationalKeys.publicKey
                                                             fabricId:@1
                                                               nodeId:@1
                                                caseAuthenticatedTags:catsWithDuplicatedCAT
                                                                error:nil];
    XCTAssertNil(operationalCert);

    // CAT with invalid version
    operationalCert = [MTRCertificates generateOperationalCertificate:rootKeys
                                                   signingCertificate:rootCert
                                                 operationalPublicKey:operationalKeys.publicKey
                                                             fabricId:@1
                                                               nodeId:@1
                                                caseAuthenticatedTags:catsWithInvalidVersion
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
