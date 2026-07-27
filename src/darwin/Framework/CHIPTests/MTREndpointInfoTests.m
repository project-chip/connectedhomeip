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
#import <Matter/Matter.h>

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

static NSArray<NSNumber *> * Exclude(NSArray<NSNumber *> * numbers, NSNumber * numberToExclude)
{
    NSMutableArray * result = [numbers mutableCopy];
    [result removeObject:numberToExclude];
    return result;
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

- (void)testPopulateChildren2
{
    // Same as testPopulateChildren, but with reversed PartsLists
    NSDictionary<NSNumber *, MTREndpointInfo *> * endpoints = [self indexEndpoints:@[
        MakeEndpoint(@0, @[ @6, @5, @4, @3, @2, @1 ]), // full-family pattern
        MakeEndpoint(@1, @[ @3, @2 ]),
        MakeEndpoint(@2, @[]),
        MakeEndpoint(@3, @[]),
        MakeEndpoint(@4, @[ @6, @5 ]), // full-family pattern
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

- (void)testPopulateChildrenInvalidCompositionCycle
{
    NSDictionary<NSNumber *, MTREndpointInfo *> * endpoints = [self indexEndpoints:@[
        MakeEndpoint(@0, @[ @1, @2, @3, @4, @5, @6 ]), // full-family pattern
        MakeEndpoint(@1, @[ @2, @3 ]),
        MakeEndpoint(@2, @[]),
        MakeEndpoint(@3, @[]),
        MakeEndpoint(@4, @[ @5, @6 ]), // full-family pattern
        MakeEndpoint(@5, @[ @6 ]),
        MakeEndpoint(@6, @[ @4 ]), // not valid per spec: cycle 4 -> 5 -> 6 -> 4
    ]];
    XCTAssertFalse([MTREndpointInfo populateChildrenForEndpoints:endpoints]);
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@0]), (@[ @1, @4 ]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@1]), (@[ @2, @3 ]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@2]), (@[]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@3]), (@[]));
    // We make no promises about child lists for endpoints involved in a cycle
}

- (void)testPopulateChildrenInvalidNonTree
{
    NSDictionary<NSNumber *, MTREndpointInfo *> * endpoints = [self indexEndpoints:@[
        MakeEndpoint(@0, @[ @1, @2, @3, @4, @5, @6 ]), // full-family pattern
        MakeEndpoint(@1, @[ @2, @3, @6 ]),
        MakeEndpoint(@2, @[]),
        MakeEndpoint(@3, @[]),
        MakeEndpoint(@4, @[ @5, @6 ]), // full-family pattern
        MakeEndpoint(@5, @[ @6 ]),
        MakeEndpoint(@6, @[]), // not valid per spec: 6 is a child of both 1 and 5
    ]];
    // Note: Not asserting a false return value here, this scenario is currently not detected.
    [MTREndpointInfo populateChildrenForEndpoints:endpoints];
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@0]), (@[ @1, @4 ]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@2]), (@[]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@3]), (@[]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@4]), (@[ @5 ]));
    // Endpoint 6 has multiple parents, so we make no guarantees where (or if) it shows up
    XCTAssertEqualObjects(Exclude(ChildEndpointIDs(endpoints[@1]), @6), (@[ @2, @3 ]));
    XCTAssertEqualObjects(Exclude(ChildEndpointIDs(endpoints[@5]), @6), (@[]));
}

- (void)testEqualityAndCopying
{
    MTRDeviceTypeRevision * doorLock = [[MTRDeviceTypeRevision alloc] initWithDeviceTypeID:@0x0A revision:@1];
    MTRDeviceTypeRevision * rootNode = [[MTRDeviceTypeRevision alloc] initWithDeviceTypeID:@0x16 revision:@1];
    MTREndpointInfo * a1 = [[MTREndpointInfo alloc] initWithEndpointID:@1 deviceTypes:@[ rootNode ] partsList:@[]];
    XCTAssertTrue([a1 isEqual:a1]);
    XCTAssertTrue([a1 isEqual:[a1 copy]]);
    XCTAssertFalse([a1 isEqual:nil]);
    XCTAssertFalse([a1 isEqual:@"hello"]);
    MTREndpointInfo * a2 = [[MTREndpointInfo alloc] initWithEndpointID:@1 deviceTypes:@[ rootNode ] partsList:@[]];
    XCTAssertTrue([a1 isEqual:a2]);
    XCTAssertTrue([a2 isEqual:a1]);
    XCTAssertEqual(a1.hash, a2.hash);
    MTREndpointInfo * b = [[MTREndpointInfo alloc] initWithEndpointID:@1 deviceTypes:@[ rootNode ] partsList:@[ @2 ]];
    XCTAssertFalse([a1 isEqual:b]);
    XCTAssertFalse([b isEqual:a1]);
    MTREndpointInfo * c = [[MTREndpointInfo alloc] initWithEndpointID:@1 deviceTypes:@[ doorLock ] partsList:@[]];
    XCTAssertFalse([a1 isEqual:c]);
    XCTAssertFalse([c isEqual:a1]);
    MTREndpointInfo * d = [[MTREndpointInfo alloc] initWithEndpointID:@2 deviceTypes:@[ rootNode ] partsList:@[]];
    XCTAssertFalse([a1 isEqual:d]);
    XCTAssertFalse([d isEqual:a1]);
}

- (void)testSecureCoding
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

    NSData * data = [NSKeyedArchiver archivedDataWithRootObject:endpoints.allValues requiringSecureCoding:YES error:NULL];
    NSArray<MTREndpointInfo *> * decodedEndpoints = [NSKeyedUnarchiver unarchivedArrayOfObjectsOfClass:MTREndpointInfo.class fromData:data error:NULL];

    XCTAssertNotNil(decodedEndpoints);
    XCTAssertEqualObjects(decodedEndpoints, endpoints.allValues);

    // Deeply compare by hand as well, `children` is not checked by isEqual:
    [decodedEndpoints enumerateObjectsUsingBlock:^(MTREndpointInfo * decoded, NSUInteger idx, BOOL * stop) {
        MTREndpointInfo * original = endpoints.allValues[idx];
        XCTAssertTrue([decoded isEqual:original]);
        XCTAssertEqualObjects(decoded.endpointID, original.endpointID);
        XCTAssertEqualObjects(decoded.deviceTypes, original.deviceTypes);
        XCTAssertEqualObjects(decoded.partsList, original.partsList);
        XCTAssertEqualObjects(decoded.children, original.children);
    }];
}

@end
