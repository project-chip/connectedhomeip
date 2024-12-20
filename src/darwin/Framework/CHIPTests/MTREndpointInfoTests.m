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

#import "MTREndpointInfo_Test.h"

#import <XCTest/XCTest.h>

@interface MTREndpointInfoTests : XCTestCase
@end

@implementation MTREndpointInfoTests

static MTREndpointInfo * MakeEndpoint(NSNumber * endpointID, NSArray<NSNumber *> * parts)
{
    return [[MTREndpointInfo alloc] initWithEndpointID:endpointID deviceTypes:@[] partsList:parts];
}

static NSArray<NSNumber *> * ChildEndpointIDs(MTREndpointInfo * endpoint)
{
    return [[endpoint.children valueForKey:@"endpointID"] sortedArrayUsingSelector:@selector(compare:)];
}

- (NSDictionary<NSNumber *, MTREndpointInfo *> *)indexEndpoints:(NSArray<MTREndpointInfo *> *)endpoints
{
    NSMutableDictionary * indexed = [[NSMutableDictionary alloc] init];
    for (MTREndpointInfo * endpoint in endpoints) {
        indexed[endpoint.endpointID] = endpoint;
    }
    XCTAssertEqual(indexed.count, endpoints.count, @"Duplicate endpoint IDs");
    return indexed;
}

- (void)testPopulateChildren
{
    NSDictionary<NSNumber *, MTREndpointInfo *> * endpoints = [self indexEndpoints:@[
        MakeEndpoint(@0, @[ @1, @2, @3, @4, @5, @6 ]), // full-family pattern
        MakeEndpoint(@1, @[ @2, @3 ]),
        MakeEndpoint(@2, @[]),
        MakeEndpoint(@3, @[]),
        MakeEndpoint(@4, @[ @5, @6 ]), // full-family pattern
        MakeEndpoint(@5, @[ @6 ]),
        MakeEndpoint(@6, @[]),
    ]];
    XCTAssertTrue([MTREndpointInfo populateChildrenForEndpoints:endpoints]);
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@0]), (@[ @1, @4 ]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@1]), (@[ @2, @3 ]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@2]), (@[]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@3]), (@[]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@4]), (@[ @5 ]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@5]), (@[ @6 ]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@6]), (@[]));
}

- (void)testPopulateChildrenRootOnly
{
    NSDictionary<NSNumber *, MTREndpointInfo *> * endpoints = [self indexEndpoints:@[
        MakeEndpoint(@0, @[]),
    ]];
    XCTAssertTrue([MTREndpointInfo populateChildrenForEndpoints:endpoints]);
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@0]), (@[]));
}

- (void)testPopulateChildrenWithCompositionCycle
{
    NSDictionary<NSNumber *, MTREndpointInfo *> * endpoints = [self indexEndpoints:@[
        MakeEndpoint(@0, @[ @1, @2, @3, @4, @5, @6 ]), // full-family pattern
        MakeEndpoint(@1, @[ @2, @3 ]),
        MakeEndpoint(@2, @[]),
        MakeEndpoint(@3, @[]),
        MakeEndpoint(@4, @[ @5, @6 ]), // full-family pattern
        MakeEndpoint(@5, @[ @6 ]),
        MakeEndpoint(@6, @[ @4 ]), // cycle 4 -> 5 -> 6 -> 4
    ]];
    XCTAssertFalse([MTREndpointInfo populateChildrenForEndpoints:endpoints]);
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@0]), (@[ @1, @4 ]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@1]), (@[ @2, @3 ]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@2]), (@[]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@3]), (@[]));
    // We make no promises about child lists for endpoints involved in a cycle
}

@end
