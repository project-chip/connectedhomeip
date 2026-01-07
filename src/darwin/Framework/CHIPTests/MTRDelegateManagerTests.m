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

#import <Matter/Matter.h>
#import <XCTest/XCTest.h>

#import "MTRDelegateManager.h"

@protocol MTRDelegateManagerTestsDelegate <NSObject>
@end

@interface MTRDelegateManagerTestsDelegateImpl : NSObject <MTRDelegateManagerTestsDelegate>
- (instancetype)initWithDeallocExpectation:(XCTestExpectation *)deallocExpectation;
@property (nonatomic, readonly, nullable) XCTestExpectation * deallocExpectation;
@end

@implementation MTRDelegateManagerTestsDelegateImpl
- (instancetype)init
{
    self = [super init];
    NSLog(@"Initializing delegate %p", self);
    return self;
}

- (instancetype)initWithDeallocExpectation:(XCTestExpectation *)deallocExpectation
{
    self = [super init];
    NSLog(@"Initializing delegate %p", self);
    _deallocExpectation = deallocExpectation;
    return self;
}

- (void)dealloc
{
    NSLog(@"Deallocating delegate %p", self);
    if (_deallocExpectation) {
        [_deallocExpectation fulfill];
    }
}
@end

@interface MTRDelegateManagerTestsDelegateInfo : MTRDelegateInfo <id <MTRDelegateManagerTestsDelegate>>
@end

@implementation MTRDelegateManagerTestsDelegateInfo
@end

@interface MTRDelegateManagerTests : XCTestCase
@end

@implementation MTRDelegateManagerTests

- (void)testBasicAddRemove
{
    MTRDelegateManager<MTRDelegateManagerTestsDelegateImpl *, MTRDelegateManagerTestsDelegateInfo *> * manager = [[MTRDelegateManager alloc] initWithOwner:self];
    XCTAssertEqual([manager unitTestNonnullDelegateCount], 0);

    __auto_type * delegate = [[MTRDelegateManagerTestsDelegateImpl alloc] init];
    __auto_type * info = [[MTRDelegateManagerTestsDelegateInfo alloc] initWithDelegate:delegate queue:dispatch_get_main_queue()];

    [manager addDelegateInfo:info];
    XCTAssertEqual([manager unitTestNonnullDelegateCount], 1);

    [manager removeDelegate:delegate];
    XCTAssertEqual([manager unitTestNonnullDelegateCount], 0);
}

- (void)testRemoveAll
{
    MTRDelegateManager<MTRDelegateManagerTestsDelegateImpl *, MTRDelegateManagerTestsDelegateInfo *> * manager = [[MTRDelegateManager alloc] initWithOwner:self];
    XCTAssertEqual([manager unitTestNonnullDelegateCount], 0);

    __auto_type * delegate1 = [[MTRDelegateManagerTestsDelegateImpl alloc] init];
    __auto_type * info1 = [[MTRDelegateManagerTestsDelegateInfo alloc] initWithDelegate:delegate1 queue:dispatch_get_main_queue()];

    __auto_type * delegate2 = [[MTRDelegateManagerTestsDelegateImpl alloc] init];
    __auto_type * info2 = [[MTRDelegateManagerTestsDelegateInfo alloc] initWithDelegate:delegate2 queue:dispatch_get_main_queue()];

    __auto_type * delegate3 = [[MTRDelegateManagerTestsDelegateImpl alloc] init];
    __auto_type * info3 = [[MTRDelegateManagerTestsDelegateInfo alloc] initWithDelegate:delegate3 queue:dispatch_get_main_queue()];

    [manager addDelegateInfo:info1];
    [manager addDelegateInfo:info2];
    [manager addDelegateInfo:info3];
    XCTAssertEqual([manager unitTestNonnullDelegateCount], 3);

    [manager removeAllDelegates];
    XCTAssertEqual([manager unitTestNonnullDelegateCount], 0);
}

- (void)testReplacingAdd
{
    MTRDelegateManager<MTRDelegateManagerTestsDelegateImpl *, MTRDelegateManagerTestsDelegateInfo *> * manager = [[MTRDelegateManager alloc] initWithOwner:self];
    XCTAssertEqual([manager unitTestNonnullDelegateCount], 0);

    __auto_type * delegate1 = [[MTRDelegateManagerTestsDelegateImpl alloc] init];
    __auto_type * info1 = [[MTRDelegateManagerTestsDelegateInfo alloc] initWithDelegate:delegate1 queue:dispatch_get_main_queue()];
    __auto_type * info2 = [[MTRDelegateManagerTestsDelegateInfo alloc] initWithDelegate:delegate1 queue:dispatch_get_main_queue()];

    __auto_type * delegate2 = [[MTRDelegateManagerTestsDelegateImpl alloc] init];
    __auto_type * info3 = [[MTRDelegateManagerTestsDelegateInfo alloc] initWithDelegate:delegate2 queue:dispatch_get_main_queue()];

    [manager addDelegateInfo:info1];
    [manager addDelegateInfo:info2];
    XCTAssertEqual([manager unitTestNonnullDelegateCount], 1);

    [manager addDelegateInfo:info3];
    XCTAssertEqual([manager unitTestNonnullDelegateCount], 2);
}

- (void)testIterate
{
    MTRDelegateManager<MTRDelegateManagerTestsDelegateImpl *, MTRDelegateManagerTestsDelegateInfo *> * manager = [[MTRDelegateManager alloc] initWithOwner:self];

    __block NSMutableSet<MTRDelegateManagerTestsDelegateInfo *> * infoSet = [NSMutableSet set];
    __auto_type addBlock = ^(MTRDelegateManagerTestsDelegateInfo * info) {
        [infoSet addObject:info];
    };

    [manager iterateDelegatesWithBlock:addBlock];
    XCTAssertEqualObjects(infoSet, [NSSet set]);

    __auto_type * delegate1 = [[MTRDelegateManagerTestsDelegateImpl alloc] init];
    __auto_type * info1 = [[MTRDelegateManagerTestsDelegateInfo alloc] initWithDelegate:delegate1 queue:dispatch_get_main_queue()];

    __auto_type * delegate3 = [[MTRDelegateManagerTestsDelegateImpl alloc] init];
    __auto_type * info3 = [[MTRDelegateManagerTestsDelegateInfo alloc] initWithDelegate:delegate3 queue:dispatch_get_main_queue()];

    MTRDelegateManagerTestsDelegateInfo * info2;
    @autoreleasepool {
        __auto_type * delegate2 = [[MTRDelegateManagerTestsDelegateImpl alloc] init];
        info2 = [[MTRDelegateManagerTestsDelegateInfo alloc] initWithDelegate:delegate2 queue:dispatch_get_main_queue()];

        [manager addDelegateInfo:info1];
        [manager addDelegateInfo:info2];
        [manager addDelegateInfo:info3];

        infoSet = [NSMutableSet set];
        [manager iterateDelegatesWithBlock:addBlock];

        __auto_type * expectedSet = [NSSet setWithArray:@[ info1, info2, info3 ]];
        XCTAssertEqualObjects(infoSet, expectedSet);
    }

    // Now delegate2 should go away.
    infoSet = [NSMutableSet set];
    [manager iterateDelegatesWithBlock:addBlock];

    __auto_type * expectedSet = [NSSet setWithArray:@[ info1, info3 ]];
    XCTAssertEqualObjects(infoSet, expectedSet);
}

- (void)testCall
{
    MTRDelegateManager<MTRDelegateManagerTestsDelegateImpl *, MTRDelegateManagerTestsDelegateInfo *> * manager = [[MTRDelegateManager alloc] initWithOwner:self];

    __auto_type * delegate2 = [[MTRDelegateManagerTestsDelegateImpl alloc] init];
    __auto_type * info2 = [[MTRDelegateManagerTestsDelegateInfo alloc] initWithDelegate:delegate2 queue:dispatch_get_main_queue()];

    __auto_type * delegate3 = [[MTRDelegateManagerTestsDelegateImpl alloc] init];
    dispatch_queue_t queue3 = dispatch_queue_create("MTRDelegateManagerTests", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
    __auto_type * info3 = [[MTRDelegateManagerTestsDelegateInfo alloc] initWithDelegate:delegate3 queue:queue3];

    MTRDelegateManagerTestsDelegateInfo * info1;
    __auto_type * delegate1DeallocExpectation = [self expectationWithDescription:@"Delegate 1 deallocated"];
    @autoreleasepool {
        __auto_type * delegate1 = [[MTRDelegateManagerTestsDelegateImpl alloc] initWithDeallocExpectation:delegate1DeallocExpectation];
        info1 = [[MTRDelegateManagerTestsDelegateInfo alloc] initWithDelegate:delegate1 queue:dispatch_get_main_queue()];

        [manager addDelegateInfo:info1];
        [manager addDelegateInfo:info2];
        [manager addDelegateInfo:info3];

        __auto_type * expectation1 = [self expectationWithDescription:@"Delegate 1"];
        __auto_type * expectation2 = [self expectationWithDescription:@"Delegate 2"];
        __auto_type * expectation3 = [self expectationWithDescription:@"Delegate 3"];
        [manager callDelegatesWithBlock:^(MTRDelegateManagerTestsDelegateImpl * delegate) {
            if (delegate == delegate1) {
                dispatch_assert_queue(dispatch_get_main_queue());
                [expectation1 fulfill];
            } else if (delegate == delegate2) {
                dispatch_assert_queue(dispatch_get_main_queue());
                [expectation2 fulfill];
            } else if (delegate == delegate3) {
                dispatch_assert_queue(queue3);
                [expectation3 fulfill];
            } else {
                XCTFail("Unexpected delegate: %@", delegate);
            }
        }];

        [self waitForExpectations:@[ expectation1, expectation3, expectation2 ] timeout:3]; // NOTE: No particular ordering required
    }

    // Now delegate1 should go away.
    [self waitForExpectations:@[ delegate1DeallocExpectation ] timeout:3];

    __auto_type * expectation2 = [self expectationWithDescription:@"Delegate 2 again"];
    __auto_type * expectation3 = [self expectationWithDescription:@"Delegate 3 again"];
    [manager callDelegatesWithBlock:^(MTRDelegateManagerTestsDelegateImpl * delegate) {
        if (delegate == delegate2) {
            dispatch_assert_queue(dispatch_get_main_queue());
            [expectation2 fulfill];
        } else if (delegate == delegate3) {
            dispatch_assert_queue(queue3);
            [expectation3 fulfill];
        } else {
            XCTFail("Unexpected delegate: %@", delegate);
        }
    }];

    [self waitForExpectations:@[ expectation3, expectation2 ] timeout:3]; // NOTE: No particular ordering required
}

@end
