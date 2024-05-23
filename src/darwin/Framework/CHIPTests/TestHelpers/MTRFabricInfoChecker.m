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

#import "MTRFabricInfoChecker.h"
#import <XCTest/XCTest.h>

void CheckFabricInfo(NSArray<MTRFabricInfo *> * fabricInfoList, NSMutableSet<NSDictionary *> * expectedSet)
{
    XCTAssertEqual([fabricInfoList count], [expectedSet count]);
    for (size_t i = 0; i < [fabricInfoList count]; ++i) {
        __auto_type * info = fabricInfoList[i];
        NSSet<NSDictionary *> * expectedInfoForIndex = [expectedSet objectsPassingTest:^(NSDictionary * obj, BOOL * stop) {
            return [info.fabricIndex isEqual:obj[@"fabricIndex"]];
        }];
        XCTAssertEqual([expectedInfoForIndex count], 1);
        __auto_type * expected = [expectedInfoForIndex anyObject];
        XCTAssertEqualObjects(info.rootPublicKey, expected[@"rootPublicKey"]);
        XCTAssertEqualObjects(info.vendorID, expected[@"vendorID"]);
        XCTAssertEqualObjects(info.fabricID, expected[@"fabricID"]);
        XCTAssertEqualObjects(info.nodeID, expected[@"nodeID"]);
        XCTAssertEqualObjects(info.label, expected[@"label"]);
        XCTAssertNotNil(info.rootCertificate);
        XCTAssertNotNil(info.rootCertificateTLV);
        XCTAssertEqualObjects([MTRCertificates convertX509Certificate:info.rootCertificate], info.rootCertificateTLV);
        XCTAssertEqualObjects([MTRCertificates convertMatterCertificate:info.rootCertificateTLV], info.rootCertificate);
        XCTAssertEqual((info.intermediateCertificate == nil), (info.intermediateCertificateTLV == nil));
        XCTAssertEqualObjects(@(info.intermediateCertificate != nil), expected[@"hasIntermediateCertificate"]);
        if (info.intermediateCertificate != nil) {
            XCTAssertEqualObjects(
                [MTRCertificates convertX509Certificate:info.intermediateCertificate], info.intermediateCertificateTLV);
            XCTAssertEqualObjects(
                [MTRCertificates convertMatterCertificate:info.intermediateCertificateTLV], info.intermediateCertificate);
        }
        XCTAssertNotNil(info.operationalCertificate);
        XCTAssertNotNil(info.operationalCertificateTLV);
        XCTAssertEqualObjects([MTRCertificates convertX509Certificate:info.operationalCertificate], info.operationalCertificateTLV);
        XCTAssertEqualObjects(
            [MTRCertificates convertMatterCertificate:info.operationalCertificateTLV], info.operationalCertificate);
        __auto_type * certInfo = [[MTRCertificateInfo alloc] initWithTLVBytes:info.operationalCertificateTLV];
        XCTAssertNotNil(certInfo);
        XCTAssertEqualObjects(certInfo.subject.nodeID, info.nodeID);
        XCTAssertEqualObjects(certInfo.subject.fabricID, info.fabricID);
        XCTAssertEqualObjects(info.fabricIndex, expected[@"fabricIndex"]);
        [expectedSet removeObject:expected];
    }
}
