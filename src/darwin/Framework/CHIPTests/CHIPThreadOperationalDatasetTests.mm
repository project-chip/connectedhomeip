//
//  CHIPControllerTests.m
//  CHIPControllerTests
/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

@interface CHIPThreadOperationalDatasetTests : XCTestCase

@end

@implementation CHIPThreadOperationalDatasetTests

- (void)testThreadOperationalDataset
{
    const unsigned char extendedPANID[] = { 0x68, 0x09, 0x45, 0x04, 0xae, 0xef, 0x42, 0x67 };
    const unsigned char masterKey[]
        = { 0x7c, 0x77, 0x08, 0x70, 0xeb, 0x05, 0xcc, 0x6d, 0xbe, 0xcc, 0x6d, 0x62, 0x32, 0xea, 0xb8, 0xb9 };
    const unsigned char PKSc[] = { 0xc4, 0xa3, 0x81, 0x25, 0x94, 0x77, 0x81, 0x99, 0x6e, 0xf5, 0x61, 0xdf, 0x8f, 0xb7, 0x8d, 0x23 };
    const uint16_t panID = 0x28f4;
    CHIPThreadOperationalDataset * dataset = [[CHIPThreadOperationalDataset alloc]
        initWithNetworkName:@"TestNetwork"
              extendedPANID:[NSData dataWithBytes:&extendedPANID length:CHIPSizeThreadExtendedPanId]
                  masterKey:[NSData dataWithBytes:&masterKey length:CHIPSizeThreadMasterKey]
                       PSKc:[NSData dataWithBytes:&PKSc length:CHIPSizeThreadPSKc]
                    channel:25
                      panID:[NSData dataWithBytes:&panID length:sizeof(panID)]];
    XCTAssertNotNil(dataset);
    NSData * data = [dataset asData];
    XCTAssertNotNil(data);

    CHIPThreadOperationalDataset * reconstructed = [[CHIPThreadOperationalDataset alloc] initWithData:data];
    XCTAssertNotNil(reconstructed);
    XCTAssertEqualObjects(reconstructed.networkName, dataset.networkName);
    XCTAssertEqualObjects(reconstructed.panID, dataset.panID);
    XCTAssertEqualObjects(reconstructed.masterKey, dataset.masterKey);
    XCTAssertEqualObjects(reconstructed.PSKc, dataset.PSKc);
    XCTAssertEqualObjects(reconstructed.extendedPANID, dataset.extendedPANID);
    XCTAssertEqual(reconstructed.channel, dataset.channel);
}

- (void)testThreadOperationalDatasetInvalid
{
    const unsigned char extendedPANID[] = { 0x67 };
    const unsigned char masterKey[] = {};
    const unsigned char PKSc[] = { 0xb7, 0x8d, 0x23 };
    const uint16_t panID = 0x0;
    CHIPThreadOperationalDataset * dataset =
        [[CHIPThreadOperationalDataset alloc] initWithNetworkName:@"TestNetwork"
                                                    extendedPANID:[NSData dataWithBytes:&extendedPANID length:sizeof(extendedPANID)]
                                                        masterKey:[NSData dataWithBytes:&masterKey length:sizeof(masterKey)]
                                                             PSKc:[NSData dataWithBytes:&PKSc length:sizeof(PKSc)]
                                                          channel:25
                                                            panID:[NSData dataWithBytes:&panID length:sizeof(panID)]];

    XCTAssertNil(dataset);
}

@end
