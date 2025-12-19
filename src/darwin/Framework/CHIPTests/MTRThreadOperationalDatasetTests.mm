//
//  MTRControllerTests.m
//  MTRControllerTests
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

#import <Matter/Matter.h>

// system dependencies
#import <XCTest/XCTest.h>

@interface MTRThreadOperationalDatasetTests : XCTestCase

@end

@implementation MTRThreadOperationalDatasetTests

- (void)testThreadOperationalDataset
{
    const unsigned char extendedPANID[] = { 0x68, 0x09, 0x45, 0x04, 0xae, 0xef, 0x42, 0x67 };
    const unsigned char masterKey[]
        = { 0x7c, 0x77, 0x08, 0x70, 0xeb, 0x05, 0xcc, 0x6d, 0xbe, 0xcc, 0x6d, 0x62, 0x32, 0xea, 0xb8, 0xb9 };
    const unsigned char PKSc[] = { 0xc4, 0xa3, 0x81, 0x25, 0x94, 0x77, 0x81, 0x99, 0x6e, 0xf5, 0x61, 0xdf, 0x8f, 0xb7, 0x8d, 0x23 };
    const uint16_t panID = 0x28f4;
    MTRThreadOperationalDataset * dataset = [[MTRThreadOperationalDataset alloc]
        initWithNetworkName:@"TestNetwork"
              extendedPANID:[NSData dataWithBytes:&extendedPANID length:MTRSizeThreadExtendedPANID]
                  masterKey:[NSData dataWithBytes:&masterKey length:MTRSizeThreadMasterKey]
                       PSKc:[NSData dataWithBytes:&PKSc length:MTRSizeThreadPSKc]
              channelNumber:@(25)
                      panID:[NSData dataWithBytes:&panID length:sizeof(panID)]];
    XCTAssertNotNil(dataset);
    NSData * data = [dataset data];
    XCTAssertNotNil(data);

    MTRThreadOperationalDataset * reconstructed = [[MTRThreadOperationalDataset alloc] initWithData:data];
    XCTAssertNotNil(reconstructed);
    XCTAssertEqualObjects(reconstructed.networkName, dataset.networkName);
    XCTAssertEqualObjects(reconstructed.panID, dataset.panID);
    XCTAssertEqualObjects(reconstructed.masterKey, dataset.masterKey);
    XCTAssertEqualObjects(reconstructed.PSKc, dataset.PSKc);
    XCTAssertEqualObjects(reconstructed.extendedPANID, dataset.extendedPANID);
    XCTAssertEqualObjects(reconstructed.channelNumber, dataset.channelNumber);
}

- (void)testThreadOperationalDatasetInvalid
{
    const unsigned char extendedPANID[] = { 0x67 };
    const unsigned char masterKey[] = {};
    const unsigned char PKSc[] = { 0xb7, 0x8d, 0x23 };
    const uint16_t panID = 0x0;
    MTRThreadOperationalDataset * dataset =
        [[MTRThreadOperationalDataset alloc] initWithNetworkName:@"TestNetwork"
                                                   extendedPANID:[NSData dataWithBytes:&extendedPANID length:sizeof(extendedPANID)]
                                                       masterKey:[NSData dataWithBytes:&masterKey length:sizeof(masterKey)]
                                                            PSKc:[NSData dataWithBytes:&PKSc length:sizeof(PKSc)]
                                                   channelNumber:@(25)
                                                           panID:[NSData dataWithBytes:&panID length:sizeof(panID)]];

    XCTAssertNil(dataset);
}

- (void)testValidDatasetFromBytes
{
    static const uint8_t kValidExampleDataset[] = {
        0x0e, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, // Active Timestamp 1
        0x00, 0x03, 0x00, 0x00, 0x0f,                               // Channel 15
        0x35, 0x04, 0x07, 0xff, 0xf8, 0x00,                         // Channel Mask 0x07fff800
        0x02, 0x08, 0x39, 0x75, 0x8e, 0xc8, 0x14, 0x4b, 0x07, 0xfb, // Ext PAN ID 39758ec8144b07fb
        0x07, 0x08, 0xfd, 0xf1, 0xf1, 0xad, 0xd0, 0x79, 0x7d, 0xc0, // Mesh Local Prefix fdf1:f1ad:d079:7dc0::/64
        0x05, 0x10, 0xf3, 0x66, 0xce, 0xc7, 0xa4, 0x46, 0xba, 0xb9, 0x78, 0xd9, 0x0d, 0x27, 0xab, 0xe3, 0x8f, 0x23, // Network Key
        0x03, 0x0f, 0x4f, 0x70, 0x65, 0x6e, 0x54, 0x68, 0x72, 0x65, 0x61, 0x64, 0x2d, 0x35, 0x39, 0x33, 0x38,       // "OpenThread-5938"
        0x01, 0x02, 0x59, 0x38,                                                                                     // PAN ID : 0x5938
        0x04, 0x10, 0x3c, 0xa6, 0x7c, 0x96, 0x9e, 0xfb, 0x0d, 0x0c, 0x74, 0xa4, 0xd8, 0xee, 0x92, 0x3b, 0x57, 0x6c, // PKSc
        0x0c, 0x04, 0x02, 0xa0, 0xf7, 0xf8,                                                                         // Security Policy
    };
    
    NSData * data = [NSData dataWithBytes:kValidExampleDataset length:sizeof(kValidExampleDataset)];
    MTRThreadOperationalDataset * dataset = [[MTRThreadOperationalDataset alloc] initWithData:data];
    XCTAssertNotNil(data);

    XCTAssertEqualObjects(dataset.channelNumber, @15);
    XCTAssertEqualObjects(dataset.networkName, @"OpenThread-5938");
    uint16_t expectedPanID = 0x5938;
    XCTAssertEqualObjects(dataset.panID, [NSData dataWithBytes:&expectedPanID length:sizeof(expectedPanID)]);
    uint8_t expectedMasterKey[] = { 0xf3, 0x66, 0xce, 0xc7, 0xa4, 0x46, 0xba, 0xb9, 0x78, 0xd9, 0x0d, 0x27, 0xab, 0xe3, 0x8f, 0x23 };
    XCTAssertEqualObjects(dataset.masterKey, [NSData dataWithBytes:expectedMasterKey length:sizeof(expectedMasterKey)]);
    uint8_t expectedPSKc[] = { 0x3c, 0xa6, 0x7c, 0x96, 0x9e, 0xfb, 0x0d, 0x0c, 0x74, 0xa4, 0xd8, 0xee, 0x92, 0x3b, 0x57, 0x6c };
    XCTAssertEqualObjects(dataset.PSKc, [NSData dataWithBytes:expectedPSKc length:sizeof(expectedPSKc)]);
    uint8_t expectedExPanID[] = { 0x39, 0x75, 0x8e, 0xc8, 0x14, 0x4b, 0x07, 0xfb };
    XCTAssertEqualObjects(dataset.extendedPANID, [NSData dataWithBytes:expectedExPanID length:sizeof(expectedExPanID)]);
}

- (void)testDatasetWithInvalidValuesFromBytes
{
    static const uint8_t kShortNetworkKeyDataset[] = {
        0x0e, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, // Active Timestamp 1
        0x00, 0x03, 0x00, 0x00, 0x0f,                               // Channel 15
        0x35, 0x04, 0x07, 0xff, 0xf8, 0x00,                         // Channel Mask 0x07fff800
        0x02, 0x08, 0x39, 0x75, 0x8e, 0xc8, 0x14, 0x4b, 0x07, 0xfb, // Ext PAN ID 39758ec8144b07fb
        0x07, 0x08, 0xfd, 0xf1, 0xf1, 0xad, 0xd0, 0x79, 0x7d, 0xc0, // Mesh Local Prefix fdf1:f1ad:d079:7dc0::/64
        0x05, 0x08, 0xf3, 0x66, 0xce, 0xc7, 0xa4, 0x46, 0xba, 0xb9, // Network Key (8 bytes, should be 16)
        0x03, 0x0f, 0x4f, 0x70, 0x65, 0x6e, 0x54, 0x68, 0x72, 0x65, 0x61, 0x64, 0x2d, 0x35, 0x39, 0x33, 0x38,       // "OpenThread-5938"
        0x01, 0x02, 0x59, 0x38,                                                                                     // PAN ID : 0x5938
        0x04, 0x10, 0x3c, 0xa6, 0x7c, 0x96, 0x9e, 0xfb, 0x0d, 0x0c, 0x74, 0xa4, 0xd8, 0xee, 0x92, 0x3b, 0x57, 0x6c, // PKSc
        0x0c, 0x04, 0x02, 0xa0, 0xf7, 0xf8,                                                                         // Security Policy
    };
    NSData * data = [NSData dataWithBytes:kShortNetworkKeyDataset length:sizeof(kShortNetworkKeyDataset)];
    MTRThreadOperationalDataset * dataset = [[MTRThreadOperationalDataset alloc] initWithData:data];
    XCTAssertNil(dataset);
}

@end
