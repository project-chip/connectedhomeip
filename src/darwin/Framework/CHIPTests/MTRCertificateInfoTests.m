//
/**
 *    Copyright (c) 2023 Project CHIP Authors
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

@interface MTRCertificateInfoTests : XCTestCase

@end

@implementation MTRCertificateInfoTests

// CHIP Certificate:
//    Signature Algo  : ECDSAWithSHA256
//    Issuer          : [[ MatterRCACId = 0000000000000001 ]]
//    Not Before      : 0x27812280  ( 2021/01/01 00:00:00 )
//    Not After       : 0x3A4D2580  ( 2030/12/30 00:00:00 )
//    Subject         : [[ MatterRCACId = 0000000000000001 ]]
//    Public Key Algo : ECPublicKey
//    Curve Id        : prime256v1
//    Public Key      : 04 61 E7 5C FE 03 EC 90 5E F4 53 77 E5 F4 B5 1D
//                      E4 3D 55 F4 D4 E7 EA 7B 22 76 5E 59 CA 0A 9B DB
//                      59 8F 69 DB 12 8B FB 35 BF 03 1F E9 51 ED 53 37
//                      5D 35 B0 53 22 F4 DF 7E BA EC 02 AC 52 F3 1B 9A
//                      D7
//    Extensions:
//        Is CA            : true
//        Key Usage        : KeyCertSign CRLSign
//        Subject Key Id   : F4 78 BD A6 C4 1B 24 4C 8A 3B 79 48 2B 1A 1C 7B AE 2E A6 0F
//        Authority Key Id : F4 78 BD A6 C4 1B 24 4C 8A 3B 79 48 2B 1A 1C 7B AE 2E A6 0F
//    Signature       : 21 B8 CF A1 6D 3B 4B 71 6B 51 91 1A 46 80 D1 92
//                      1E 6D 3F 65 F2 A5 A4 27 67 7E 04 0A 50 99 96 2B
//                      20 A4 11 10 7D 18 88 51 DD 30 84 52 A2 AA 40 3D
//                      EB 72 53 80 B0 6C 97 95 FF 38 66 38 F7 A9 65 F1

- (MTRCertificateTLVBytes)exampleRCACertTLV
{
    return [[NSData alloc]
        initWithBase64EncodedString:
            @"FTABAQEkAgE3AyQUARgmBIAigScmBYAlTTo3BiQUARgkBwEkCAEwCUEEYedc/gPskF70U3fl9"
             "LUd5D1V9NTn6nsidl5Zygqb21mPadsSi/s1vwMf6VHtUzddNbBTIvTffrrsAqxS8xua1zcKNQEpARgkAmAwBBT0eL2mxBskTIo7eUgrGhx7ri6mDzAFFP"
             "R4vabEGyRMijt5SCsaHHuuLqYPGDALQCG4z6FtO0txa1GRGkaA0ZIebT9l8qWkJ2d+BApQmZYrIKQREH0YiFHdMIRSoqpAPetyU4CwbJeV/zhmOPepZfE"
             "Y"
                            options:0];
}

- (void)testParseRootCertificateTLV
{
    MTRCertificateInfo * info = [[MTRCertificateInfo alloc] initWithTLVBytes:self.exampleRCACertTLV];
    XCTAssertNotNil(info);
    XCTAssertEqual([NSDate.now compare:info.notBefore], NSOrderedDescending);
    XCTAssertEqual([NSDate.now compare:info.notAfter], NSOrderedAscending);

    MTRDistinguishedNameInfo * subject = info.subject;
    XCTAssertNotNil(subject);
    XCTAssertEqualObjects(subject.rootCACertificateID, @1);
    XCTAssertNil(subject.fabricID);
    XCTAssertNil(subject.nodeID);
    XCTAssertEqualObjects(subject.caseAuthenticatedTags, [NSSet set]);

    XCTAssertEqualObjects(info.issuer, subject);
}

// CHIP Certificate:
//    Signature Algo  : ECDSAWithSHA256
//    Issuer          : [[ MatterRCACId = 0000000000000001 ]]
//    Not Before      : 0x2B61DC00  ( 2023/01/24 00:00:00 )
//    Not After       : 0x3E2DDEFF  ( 2033/01/20 23:59:59 )
//    Subject         : [[ MatterFabricId = 0000000000000001,
//                         MatterNodeId = 000000000001B669,
//                         MatterCASEAuthTag = 00010001,
//                         MatterCASEAuthTag = 00080001 ]]
//    Public Key Algo : ECPublicKey
//    Curve Id        : prime256v1
//    Public Key      : 04 FD 7F 7A 19 37 E0 6D A4 C2 C9 95 C5 5F 01 10
//                      D2 B0 FC 93 50 4C 4A 2A D5 65 90 8B 42 7C AF 60
//                      4A 1C 24 F9 C1 75 61 4C 45 DC 87 34 0F B4 1C 24
//                      D2 40 11 3F 82 6D 8B 2A 24 0E E4 A5 9B 30 C6 A9
//                      51
//    Extensions:
//        Is CA            : false
//        Key Usage        : DigitalSignature
//        Key Purpose      : ServerAuth ClientAuth
//        Subject Key Id   : 03 FC 90 70 10 72 B7 55 09 30 59 B1 B9 4F 1C AE 44 3F DD 1C
//        Authority Key Id : 16 49 0D 1A 14 45 78 D1 B8 38 F2 8D 81 0E 3F 5E 66 83 2F DD
//    Signature       : 77 2B 56 33 8B 86 AC AC 39 45 C4 0D 70 05 09 CD
//                      46 E2 B7 17 A2 62 94 04 88 50 92 1C 65 53 5A 34
//                      01 98 EB 07 07 5E DC BF A1 BE 15 08 95 2B 7A 07
//                      03 21 59 7C E4 DD 0E F7 05 14 A5 78 B1 98 91 73

- (MTRCertificateTLVBytes)exampleNOCertTLV
{
    return [[NSData alloc]
        initWithBase64EncodedString:
            @"FTABCAvuIv83A5WWJAIBNwMkFAEYJgQA3GErJgX/3i0+NwYkFQEmEWm2AQAmFgEAAQAmFgEAC"
             "AAYJAcBJAgBMAlBBP1/ehk34G2kwsmVxV8BENKw/JNQTEoq1WWQi0J8r2BKHCT5wXVhTEXchzQPtBwk0kARP4JtiyokDuSlmzDGqVE3CjUBKAEYJAIBNg"
             "MEAgQBGDAEFAP8kHAQcrdVCTBZsblPHK5EP90cMAUUFkkNGhRFeNG4OPKNgQ4/XmaDL90YMAtAdytWM4uGrKw5RcQNcAUJzUbitxeiYpQEiFCSHGVTWjQ"
             "BmOsHB17cv6G+FQiVK3oHAyFZfOTdDvcFFKV4sZiRcxg="
                            options:0];
}

- (void)testParseOperationalCertificateTLV
{
    MTRCertificateInfo * info = [[MTRCertificateInfo alloc] initWithTLVBytes:self.exampleNOCertTLV];
    XCTAssertNotNil(info);
    XCTAssertEqual([NSDate.now compare:info.notBefore], NSOrderedDescending);
    XCTAssertEqual([NSDate.now compare:info.notAfter], NSOrderedAscending);

    MTRDistinguishedNameInfo * subject = info.subject;
    XCTAssertNotNil(subject);
    XCTAssertEqualObjects(subject.fabricID, @0x0000000000000001);
    XCTAssertEqualObjects(subject.nodeID, @0x000000000001B669);
    XCTAssertEqualObjects(subject.caseAuthenticatedTags, ([NSSet setWithObjects:@0x00010001, @0x00080001, nil]));
    XCTAssertNil(subject.rootCACertificateID);

    XCTAssertNotEqualObjects(info.issuer, subject);
}

- (void)testCertificateInfoEquality
{
    MTRCertificateInfo * infoA1 = [[MTRCertificateInfo alloc] initWithTLVBytes:self.exampleRCACertTLV];
    MTRCertificateInfo * infoA2 = [[MTRCertificateInfo alloc] initWithTLVBytes:self.exampleRCACertTLV];
    MTRCertificateInfo * infoB = [[MTRCertificateInfo alloc] initWithTLVBytes:self.exampleNOCertTLV];
    XCTAssertNotIdentical(infoA1, infoA2);
    XCTAssertEqualObjects(infoA1, infoA2);
    XCTAssertEqualObjects(infoA1, infoA1);
    XCTAssertNotEqualObjects(infoA1, infoB);
}

@end
