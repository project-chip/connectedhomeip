/**
 *    Copyright (c) 2022 Project CHIP Authors
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

#import "MTRAsyncCallbackWorkQueue_Internal.h"

@interface MTRAsyncCallbackQueueTests : XCTestCase

@end

@implementation MTRAsyncCallbackQueueTests

- (void)testRunItem
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"Work item called"];

    MTRAsyncCallbackWorkQueue * workQueue = [[MTRAsyncCallbackWorkQueue alloc] initWithContext:nil queue:dispatch_get_main_queue()];

    MTRAsyncCallbackQueueWorkItem * workItem1 =
        [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    __block int counter = 0;
    MTRAsyncCallbackReadyHandler readyHandler = ^(MTRDevice * _Nonnull device, NSUInteger retryCount) {
        counter++;
        [expectation fulfill];
        [workItem1 endWork];
    };
    workItem1.readyHandler = readyHandler;
    workItem1.cancelHandler = ^{
    };
    [workQueue enqueueWorkItem:workItem1];

    // Check for leaks.
    MTRAsyncCallbackQueueWorkItem * __weak weakItem = workItem1;
    [self addTeardownBlock:^() {
        XCTAssertNil(weakItem);
    }];

    [self waitForExpectationsWithTimeout:5 handler:nil];

    // see that it only ran once
    XCTAssertEqual(counter, 1);
}

- (void)testRunItemsSerialized
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"Work item called in order"];

    MTRAsyncCallbackWorkQueue * workQueue = [[MTRAsyncCallbackWorkQueue alloc] initWithContext:nil queue:dispatch_get_main_queue()];

    MTRAsyncCallbackQueueWorkItem * workItem1 =
        [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    __block int counter = 0;
    MTRAsyncCallbackReadyHandler readyHandler1 = ^(MTRDevice * _Nonnull device, NSUInteger retryCount) {
        sleep(1);
        counter++;
        [workItem1 endWork];
    };
    workItem1.readyHandler = readyHandler1;
    workItem1.cancelHandler = ^{
    };
    [workQueue enqueueWorkItem:workItem1];

    MTRAsyncCallbackQueueWorkItem * workItem2 =
        [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    MTRAsyncCallbackReadyHandler readyHandler2 = ^(MTRDevice * _Nonnull device, NSUInteger retryCount) {
        // expect this to have waited until workItem1's sleep(1) finished and incremented counter
        if (counter == 1) {
            [expectation fulfill];
        }
        [workItem2 endWork];
    };
    workItem2.readyHandler = readyHandler2;
    workItem2.cancelHandler = ^{
    };
    [workQueue enqueueWorkItem:workItem2];

    [self waitForExpectationsWithTimeout:5 handler:nil];

    // see that workItem1 only ran once
    XCTAssertEqual(counter, 1);
}

- (void)testRunItemsRetry
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"Work item called in order"];

    MTRAsyncCallbackWorkQueue * workQueue = [[MTRAsyncCallbackWorkQueue alloc] initWithContext:nil queue:dispatch_get_main_queue()];

    MTRAsyncCallbackQueueWorkItem * workItem1 =
        [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    __block int counter = 0;
    MTRAsyncCallbackReadyHandler readyHandler1 = ^(MTRDevice * _Nonnull device, NSUInteger retryCount) {
        sleep(1);
        counter++;

        if (retryCount) {
            // only end after retried once
            [workItem1 endWork];
        } else {
            [workItem1 retryWork];
        }
    };
    workItem1.readyHandler = readyHandler1;
    workItem1.cancelHandler = ^{
    };
    [workQueue enqueueWorkItem:workItem1];

    MTRAsyncCallbackQueueWorkItem * workItem2 =
        [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    MTRAsyncCallbackReadyHandler readyHandler2 = ^(MTRDevice * _Nonnull device, NSUInteger retryCount) {
        // expect this to have waited until workItem1's sleep(1) finished and incremented counter twice
        if (counter == 2) {
            [expectation fulfill];
        }
        [workItem2 endWork];
    };
    workItem2.readyHandler = readyHandler2;
    workItem2.cancelHandler = ^{
    };
    [workQueue enqueueWorkItem:workItem2];

    [self waitForExpectationsWithTimeout:5 handler:nil];

    // see that workItem1 ran twice after the retry
    XCTAssertEqual(counter, 2);
}

- (void)testRunItemsAfterDrain
{
    XCTestExpectation * expectation1 = [self expectationWithDescription:@"First work item caled"];
    XCTestExpectation * expectation2 = [self expectationWithDescription:@"Second work item called after drain"];

    MTRAsyncCallbackWorkQueue * workQueue = [[MTRAsyncCallbackWorkQueue alloc] initWithContext:nil queue:dispatch_get_main_queue()];

    MTRAsyncCallbackQueueWorkItem * workItem1 =
        [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    MTRAsyncCallbackReadyHandler readyHandler1 = ^(MTRDevice * _Nonnull device, NSUInteger retryCount) {
        [workItem1 endWork];
        [expectation1 fulfill];
    };
    workItem1.readyHandler = readyHandler1;
    workItem1.cancelHandler = ^{
    };
    [workQueue enqueueWorkItem:workItem1];

    [self waitForExpectations:@[ expectation1 ] timeout:5];

    MTRAsyncCallbackQueueWorkItem * workItem2 =
        [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    MTRAsyncCallbackReadyHandler readyHandler2 = ^(MTRDevice * _Nonnull device, NSUInteger retryCount) {
        [expectation2 fulfill];
        [workItem2 endWork];
    };
    workItem2.readyHandler = readyHandler2;
    workItem2.cancelHandler = ^{
    };
    [workQueue enqueueWorkItem:workItem2];

    [self waitForExpectationsWithTimeout:5 handler:nil];
}

- (void)testRunItemNoHandlers
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"Work item called"];

    MTRAsyncCallbackWorkQueue * workQueue = [[MTRAsyncCallbackWorkQueue alloc] initWithContext:nil queue:dispatch_get_main_queue()];

    MTRAsyncCallbackQueueWorkItem * workItem1 =
        [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    MTRAsyncCallbackQueueWorkItem * workItem2 =
        [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];

    __block int counter = 0;
    MTRAsyncCallbackReadyHandler readyHandler = ^(MTRDevice * _Nonnull device, NSUInteger retryCount) {
        counter++;
        [workItem2 endWork];
        [expectation fulfill];
    };
    workItem2.readyHandler = readyHandler;
    workItem2.cancelHandler = ^{
    };

    // Check that trying to run workItem1 does not crash.
    [workQueue enqueueWorkItem:workItem1];
    [workQueue enqueueWorkItem:workItem2];

    [self waitForExpectationsWithTimeout:5 handler:nil];

    // see that it only ran once
    XCTAssertEqual(counter, 1);
}

- (void)testInvalidation
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"Work item called"];
    XCTestExpectation * cancelExpectation = [self expectationWithDescription:@"Work item canceled"];

    MTRAsyncCallbackWorkQueue * workQueue = [[MTRAsyncCallbackWorkQueue alloc] initWithContext:nil queue:dispatch_get_main_queue()];

    MTRAsyncCallbackQueueWorkItem * workItem1 =
        [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    MTRAsyncCallbackReadyHandler readyHandler1 = ^(MTRDevice * _Nonnull device, NSUInteger retryCount) {
        // Give the code enqueing the other items a chance to run, so they can
        // actually get canceled.
        sleep(1);
        [workQueue invalidate];
        [workItem1 endWork];
        [expectation fulfill];
    };
    workItem1.readyHandler = readyHandler1;
    // No cancel handler on purpose.
    [workQueue enqueueWorkItem:workItem1];

    MTRAsyncCallbackQueueWorkItem * workItem2 =
        [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    MTRAsyncCallbackReadyHandler readyHandler2 = ^(MTRDevice * _Nonnull device, NSUInteger retryCount) {
        // This should never get called.
        XCTAssertFalse(YES);
        [workItem2 endWork];
    };
    workItem2.readyHandler = readyHandler2;
    // No cancel handler on purpose.
    [workQueue enqueueWorkItem:workItem2];

    MTRAsyncCallbackQueueWorkItem * workItem3 =
        [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    MTRAsyncCallbackReadyHandler readyHandler3 = ^(MTRDevice * _Nonnull device, NSUInteger retryCount) {
        // This should never get called.
        XCTAssertFalse(YES);
        [workItem3 endWork];
    };
    dispatch_block_t cancelHandler3 = ^() {
        [cancelExpectation fulfill];
    };
    workItem3.readyHandler = readyHandler3;
    workItem3.cancelHandler = cancelHandler3;
    [workQueue enqueueWorkItem:workItem3];

    [self waitForExpectations:@[ expectation, cancelExpectation ] timeout:5];
}

- (void)testBatching
{
    XCTestExpectation * workItem1ReadyExpectation = [self expectationWithDescription:@"Work item 1 called"];
    __block BOOL workItem2BatchingCalled = NO;
    __block BOOL workItem2ReadyCalled = NO;
    XCTestExpectation * workItem3ReadyExpectation = [self expectationWithDescription:@"Work item 3 called"];

    MTRAsyncCallbackWorkQueue * workQueue = [[MTRAsyncCallbackWorkQueue alloc] initWithContext:nil queue:dispatch_get_main_queue()];

    // Have a work item sleep so the testing items can queue
    MTRAsyncCallbackQueueWorkItem * workItem0 =
        [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    MTRAsyncCallbackReadyHandler readyHandler0 = ^(MTRDevice * _Nonnull device, NSUInteger retryCount) {
        // While processing item 0, enqueue additional items to test batching
        MTRAsyncCallbackQueueWorkItem * workItem1 =
            [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
        MTRAsyncCallbackReadyHandler readyHandler1 = ^(MTRDevice * _Nonnull device, NSUInteger retryCount) {
            [workItem1ReadyExpectation fulfill];
            [workItem1 endWork];
        };
        workItem1.readyHandler = readyHandler1;
        [workItem1 setBatchingID:1
                            data:@(1)
                         handler:^(id _Nonnull opaqueDataFirst, id _Nonnull opaqueDataSecond, BOOL * _Nonnull fullyMerged) {
                             XCTAssertEqualObjects(opaqueDataFirst, @(1));
                             XCTAssertEqualObjects(opaqueDataSecond, @(2));
                             *fullyMerged = YES;
                         }];
        // No cancel handler on purpose.
        [workQueue enqueueWorkItem:workItem1];

        MTRAsyncCallbackQueueWorkItem * workItem2 =
            [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
        MTRAsyncCallbackReadyHandler readyHandler2 = ^(MTRDevice * _Nonnull device, NSUInteger retryCount) {
            workItem2ReadyCalled = YES;
            [workItem2 endWork];
        };
        workItem2.readyHandler = readyHandler2;
        [workItem2 setBatchingID:1
                            data:@(2)
                         handler:^(id _Nonnull opaqueDataFirst, id _Nonnull opaqueDataSecond, BOOL * _Nonnull fullyMerged) {
                             workItem2BatchingCalled = YES;
                         }];
        // No cancel handler on purpose.
        [workQueue enqueueWorkItem:workItem2];

        MTRAsyncCallbackQueueWorkItem * workItem3 =
            [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
        MTRAsyncCallbackReadyHandler readyHandler3 = ^(MTRDevice * _Nonnull device, NSUInteger retryCount) {
            [workItem3ReadyExpectation fulfill];
            [workItem3 endWork];
        };
        workItem3.readyHandler = readyHandler3;
        [workQueue enqueueWorkItem:workItem3];

        [workItem0 endWork];
    };
    workItem0.readyHandler = readyHandler0;
    // No cancel handler on purpose.
    [workQueue enqueueWorkItem:workItem0];

    [self waitForExpectations:@[ workItem1ReadyExpectation, workItem3ReadyExpectation ] timeout:5];

    XCTAssertFalse(workItem2BatchingCalled);
    XCTAssertFalse(workItem2ReadyCalled);
}

- (void)testDuplicate
{
    XCTestExpectation * workItem0ReadyExpectation = [self expectationWithDescription:@"Work item 0 called"];
    XCTestExpectation * workItem6ReadyExpectation = [self expectationWithDescription:@"Work item 6 called"];

    MTRAsyncCallbackWorkQueue * workQueue = [[MTRAsyncCallbackWorkQueue alloc] initWithContext:nil queue:dispatch_get_main_queue()];

    // Have a work item sleep so the testing items can queue
    MTRAsyncCallbackQueueWorkItem * workItem0 =
        [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    MTRAsyncCallbackReadyHandler readyHandler0 = ^(MTRDevice * _Nonnull device, NSUInteger retryCount) {
        // While processing item 0, enqueue additional items to test duplicate checking
        MTRAsyncCallbackQueueWorkItem * workItem1 =
            [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
        MTRAsyncCallbackReadyHandler readyHandler1 = ^(MTRDevice * _Nonnull device, NSUInteger retryCount) {
            [workItem1 endWork];
        };
        workItem1.readyHandler = readyHandler1;
        [workItem1 setDuplicateTypeID:1
                              handler:^(id _Nonnull opaqueItemData, BOOL * _Nonnull isDuplicate, BOOL * stop) {
                                  if ([opaqueItemData isEqual:@(1)]) {
                                      *isDuplicate = YES;
                                      *stop = YES;
                                  } else {
                                      *stop = NO;
                                  }
                              }];
        [workQueue enqueueWorkItem:workItem1];

        MTRAsyncCallbackQueueWorkItem * workItem2 =
            [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
        MTRAsyncCallbackReadyHandler readyHandler2 = ^(MTRDevice * _Nonnull device, NSUInteger retryCount) {
            [workItem2 endWork];
        };
        workItem2.readyHandler = readyHandler2;
        [workItem2 setDuplicateTypeID:1
                              handler:^(id _Nonnull opaqueItemData, BOOL * _Nonnull isDuplicate, BOOL * stop) {
                                  if ([opaqueItemData isEqual:@(2)]) {
                                      *isDuplicate = YES;
                                      *stop = YES;
                                  } else {
                                      *stop = NO;
                                  }
                              }];
        [workQueue enqueueWorkItem:workItem2];

        MTRAsyncCallbackQueueWorkItem * workItem3 =
            [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
        MTRAsyncCallbackReadyHandler readyHandler3 = ^(MTRDevice * _Nonnull device, NSUInteger retryCount) {
            [workItem3 endWork];
        };
        workItem3.readyHandler = readyHandler3;
        [workItem3 setDuplicateTypeID:2
                              handler:^(id _Nonnull opaqueItemData, BOOL * _Nonnull isDuplicate, BOOL * stop) {
                                  if ([opaqueItemData isEqual:@(1)]) {
                                      *isDuplicate = YES;
                                      *stop = YES;
                                  } else {
                                      *stop = NO;
                                  }
                              }];
        [workQueue enqueueWorkItem:workItem3];

        // At this point we should have duplicate type 1 with data @(1) and @(2), and type 2 with data @(1).
        XCTAssertTrue([workQueue isDuplicateForTypeID:1 workItemData:@(1)]);
        XCTAssertTrue([workQueue isDuplicateForTypeID:1 workItemData:@(2)]);
        XCTAssertTrue([workQueue isDuplicateForTypeID:2 workItemData:@(1)]);

        XCTAssertFalse([workQueue isDuplicateForTypeID:0 workItemData:@(1)]);
        XCTAssertFalse([workQueue isDuplicateForTypeID:0 workItemData:@(2)]);
        XCTAssertFalse([workQueue isDuplicateForTypeID:1 workItemData:@(0)]);
        XCTAssertFalse([workQueue isDuplicateForTypeID:1 workItemData:@(3)]);
        XCTAssertFalse([workQueue isDuplicateForTypeID:2 workItemData:@(2)]);

        // Test returning *isDuplicate=NO and queuing one extra duplicate item, and that the extra item runs

        // First have a regular item with ID/data == 3/1
        MTRAsyncCallbackQueueWorkItem * workItem4 =
            [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
        MTRAsyncCallbackReadyHandler readyHandler4 = ^(MTRDevice * _Nonnull device, NSUInteger retryCount) {
            [workItem4 endWork];
        };
        workItem4.readyHandler = readyHandler4;
        [workItem4 setDuplicateTypeID:3
                              handler:^(id _Nonnull opaqueItemData, BOOL * _Nonnull isDuplicate, BOOL * stop) {
                                  if ([opaqueItemData isEqual:@(1)]) {
                                      *isDuplicate = YES;
                                      *stop = YES;
                                  } else {
                                      *stop = NO;
                                  }
                              }];
        [workQueue enqueueWorkItem:workItem4];

        XCTAssertTrue([workQueue isDuplicateForTypeID:3 workItemData:@(1)]);

        // Have a barrier item with ID/data == 3/1 that returns *isDuplicate=NO
        MTRAsyncCallbackQueueWorkItem * workItem5 =
            [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
        MTRAsyncCallbackReadyHandler readyHandler5 = ^(MTRDevice * _Nonnull device, NSUInteger retryCount) {
            [workItem5 endWork];
        };
        workItem5.readyHandler = readyHandler5;
        [workItem5 setDuplicateTypeID:3
                              handler:^(id _Nonnull opaqueItemData, BOOL * _Nonnull isDuplicate, BOOL * stop) {
                                  if ([opaqueItemData isEqual:@(1)]) {
                                      *isDuplicate = NO;
                                      *stop = YES;
                                  } else {
                                      *stop = NO;
                                  }
                              }];
        [workQueue enqueueWorkItem:workItem5];

        // After the above, the same ID/data should no longer be considered duplicate
        XCTAssertFalse([workQueue isDuplicateForTypeID:3 workItemData:@(1)]);

        // Now add regular regular item with ID/data == 3/1
        MTRAsyncCallbackQueueWorkItem * workItem6 =
            [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
        MTRAsyncCallbackReadyHandler readyHandler6 = ^(MTRDevice * _Nonnull device, NSUInteger retryCount) {
            [workItem6 endWork];
            [workItem6ReadyExpectation fulfill];
        };
        workItem6.readyHandler = readyHandler6;
        [workItem6 setDuplicateTypeID:3
                              handler:^(id _Nonnull opaqueItemData, BOOL * _Nonnull isDuplicate, BOOL * stop) {
                                  if ([opaqueItemData isEqual:@(1)]) {
                                      *isDuplicate = YES;
                                      *stop = YES;
                                  } else {
                                      *stop = NO;
                                  }
                              }];
        [workQueue enqueueWorkItem:workItem6];

        // After the above, the same ID/data should no longer be considered duplicate
        XCTAssertTrue([workQueue isDuplicateForTypeID:3 workItemData:@(1)]);

        [workItem0 endWork];
        [workItem0ReadyExpectation fulfill];
    };
    workItem0.readyHandler = readyHandler0;
    [workQueue enqueueWorkItem:workItem0];

    [self waitForExpectations:@[ workItem0ReadyExpectation, workItem6ReadyExpectation ] timeout:5];
}

@end
