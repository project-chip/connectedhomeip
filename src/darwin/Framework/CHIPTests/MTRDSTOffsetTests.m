/*
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

// module headers
#import "MTRTimeUtils.h"
#import <Matter/Matter.h>

// system dependencies
#import <XCTest/XCTest.h>

@interface MTRDSTOffsetTests : XCTestCase
@end

@implementation MTRDSTOffsetTests

- (void)test001_SingleOffset
{
    __auto_type * offsets = MTRComputeDSTOffsets(1);
    // We should be able to get offsets.
    XCTAssertNotNil(offsets);

    // And there is always at least one, even if all it says is "no offset, forever".
    XCTAssertEqual(offsets.count, 1);
    XCTAssertEqualObjects(offsets[0].validStarting, @(0));
}

- (void)test002_TryGetting2Offsets
{
    __auto_type * offsets = MTRComputeDSTOffsets(2);
    // We should be able to get offsets.
    XCTAssertNotNil(offsets);

    // And there is always at least one, even if all it says is "no offset,
    // forever".  And we should not get too many offsets.
    XCTAssertTrue(offsets.count >= 1);
    XCTAssertTrue(offsets.count <= 2);
    NSNumber * previousValidUntil = @(0);
    for (MTRTimeSynchronizationClusterDSTOffsetStruct * offset in offsets) {
        XCTAssertEqualObjects(previousValidUntil, offset.validStarting);
        previousValidUntil = offset.validUntil;
    }
}

@end
