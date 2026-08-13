/**
 *    Copyright (c) 2025 Project CHIP Authors
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

#import "MTRSecureCodingTestHelpers.h"

#import <XCTest/XCTest.h>

id _Nullable RoundTripEncodable(id<NSSecureCoding> encodable, NSError * __autoreleasing * decodeError)
{
    // We know all our encodables are in fact NSObject.
    NSObject * obj = (NSObject *) encodable;

    NSError * encodeError;
    NSData * encodedData = [NSKeyedArchiver archivedDataWithRootObject:encodable requiringSecureCoding:YES error:&encodeError];
    XCTAssertNil(encodeError, @"Failed to encode %@", NSStringFromClass(obj.class));
    XCTAssertNotNil(encodedData);

    id decodedValue = [NSKeyedUnarchiver unarchivedObjectOfClass:obj.class fromData:encodedData error:decodeError];
    if (decodedValue != nil) {
        XCTAssertTrue([decodedValue isKindOfClass:obj.class], @"Expected %@ but got %@", NSStringFromClass(obj.class), NSStringFromClass([decodedValue class]));
    }
    return decodedValue;
}
