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

@interface MTRProductIdentityTests : XCTestCase

@end

@implementation MTRProductIdentityTests

- (void)testEqualityAndCopying
{
    MTRProductIdentity * a1 = [[MTRProductIdentity alloc] initWithVendorID:@1 productID:@1];
    XCTAssertNotNil(a1);
    XCTAssertTrue([a1 isEqual:a1]);
    XCTAssertTrue([a1 isEqual:[a1 copy]]);
    XCTAssertFalse([a1 isEqual:nil]);
    XCTAssertFalse([a1 isEqual:@1]);

    MTRProductIdentity * a2 = [[MTRProductIdentity alloc] initWithVendorID:@1 productID:@1];
    XCTAssertNotNil(a2);
    XCTAssertTrue([a1 isEqual:a2]);
    XCTAssertTrue([a2 isEqual:a1]);

    MTRProductIdentity * b = [[MTRProductIdentity alloc] initWithVendorID:@1 productID:@555];
    XCTAssertNotNil(b);
    XCTAssertFalse([b isEqual:a1]);
    XCTAssertFalse([a1 isEqual:b]);

    MTRProductIdentity * c = [[MTRProductIdentity alloc] initWithVendorID:@555 productID:@1];
    XCTAssertNotNil(c);
    XCTAssertFalse([c isEqual:a1]);
    XCTAssertFalse([a1 isEqual:c]);
    XCTAssertFalse([c isEqual:b]);
    XCTAssertFalse([b isEqual:c]);
}

- (void)testSecureCoding
{
    MTRProductIdentity * a = [[MTRProductIdentity alloc] initWithVendorID:@123 productID:@42];
    NSData * data = [NSKeyedArchiver archivedDataWithRootObject:a requiringSecureCoding:YES error:NULL];
    MTRProductIdentity * b = [NSKeyedUnarchiver unarchivedObjectOfClass:MTRProductIdentity.class fromData:data error:NULL];
    XCTAssertNotNil(b);
    XCTAssertEqualObjects(b.vendorID, a.vendorID);
    XCTAssertEqualObjects(b.productID, a.productID);
    XCTAssertTrue([b isEqual:a]);
}

@end
