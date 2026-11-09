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
#import <XCTest/XCTest.h>
#import <os/lock.h>

#import "MTRAsyncWorkQueue.h"

@interface MTRAsyncWorkQueueTests : XCTestCase
@end

@implementation MTRAsyncWorkQueueTests {
    dispatch_queue_t _backgroundQueue;
}

- (dispatch_queue_t)backgroundQueue
{
    if (!_backgroundQueue) {
        _backgroundQueue = dispatch_queue_create("background queue", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
    }
    return _backgroundQueue;
}

- (void)testRunItem
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"Work item called"];

    id context = [[NSObject alloc] init];
    MTRAsyncWorkQueue * workQueue = [[MTRAsyncWorkQueue alloc] initWithContext:context];

    MTRAsyncWorkItem * workItem1 = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    __block int counter = 0;
    workItem1.readyHandler = ^(id handlerContext, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        XCTAssertIdentical(handlerContext, context);
        XCTAssertEqual(retryCount, 0);
        XCTAssertTrue(completion(MTRAsyncWorkComplete));
        counter++;
        [expectation fulfill];
    };
    [workQueue enqueueWorkItem:workItem1 description:@"work item 1"];

    // Check for leaks.
    MTRAsyncWorkItem * __weak weakItem = workItem1;
    [self addTeardownBlock:^() {
        XCTAssertNil(weakItem);
    }];

    [self waitForExpectationsWithTimeout:1 handler:nil];

    // see that it only ran once
    XCTAssertEqual(counter, 1);
}

- (void)testRunItemsSerialized
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"Work item called in order"];

    MTRAsyncWorkQueue * workQueue = [[MTRAsyncWorkQueue alloc] initWithContext:NSNull.null];

    MTRAsyncWorkItem * workItem1 = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    __block int counter = 0;
    workItem1.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        sleep(1);
        counter++;
        completion(MTRAsyncWorkComplete);
    };
    [workQueue enqueueWorkItem:workItem1 descriptionWithFormat:@"work item %d", 1];

    MTRAsyncWorkItem * workItem2 = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    workItem2.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        // expect this to have waited until workItem1's sleep(1) finished and incremented counter
        if (counter == 1) {
            [expectation fulfill];
        }
        completion(MTRAsyncWorkComplete);
    };
    [workQueue enqueueWorkItem:workItem2 description:@"work item 2"];

    [self waitForExpectationsWithTimeout:2 handler:nil];

    // see that workItem1 only ran once
    XCTAssertEqual(counter, 1);
}

- (void)testRunItemsRetry
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"Work item called in order"];

    MTRAsyncWorkQueue * workQueue = [[MTRAsyncWorkQueue alloc] initWithContext:NSNull.null];

    MTRAsyncWorkItem * workItem1 = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    __block int counter = 0;
    workItem1.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        sleep(1);
        counter++;

        if (retryCount) {
            // only end after retried once
            completion(MTRAsyncWorkComplete);
        } else {
            completion(MTRAsyncWorkNeedsRetry);
        }
    };
    [workQueue enqueueWorkItem:workItem1 description:@"needs a retry"];

    MTRAsyncWorkItem * workItem2 = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    workItem2.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        // expect this to have waited until workItem1's sleep(1) finished and incremented counter twice
        if (counter == 2) {
            [expectation fulfill];
        }
        completion(MTRAsyncWorkComplete);
    };
    [workQueue enqueueWorkItem:workItem2];

    [self waitForExpectationsWithTimeout:3 handler:nil];

    // see that workItem1 ran twice after the retry
    XCTAssertEqual(counter, 2);
}

- (void)testRunItemsAfterDrain
{
    XCTestExpectation * expectation1 = [self expectationWithDescription:@"First work item caled"];
    XCTestExpectation * expectation2 = [self expectationWithDescription:@"Second work item called after drain"];

    MTRAsyncWorkQueue * workQueue = [[MTRAsyncWorkQueue alloc] initWithContext:NSNull.null];

    MTRAsyncWorkItem * workItem1 = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    workItem1.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        completion(MTRAsyncWorkComplete);
        [expectation1 fulfill];
    };
    [workQueue enqueueWorkItem:workItem1 description:@"workItem1"];

    [self waitForExpectations:@[ expectation1 ] timeout:2];

    MTRAsyncWorkItem * workItem2 = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    workItem2.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        [expectation2 fulfill];
        completion(MTRAsyncWorkComplete);
    };
    [workQueue enqueueWorkItem:workItem2 description:@"workItem2"];

    [self waitForExpectationsWithTimeout:2 handler:nil];
}

- (void)testRunItemNoHandlers
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"Work item called"];

    MTRAsyncWorkQueue * workQueue = [[MTRAsyncWorkQueue alloc] initWithContext:NSNull.null];

    MTRAsyncWorkItem * workItem1 = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    MTRAsyncWorkItem * workItem2 = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];

    __block int counter = 0;
    workItem2.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        counter++;
        completion(MTRAsyncWorkComplete);
        [expectation fulfill];
    };

    // Check that trying to run workItem1 does not crash.
    [workQueue enqueueWorkItem:workItem1];
    [workQueue enqueueWorkItem:workItem2];

    [self waitForExpectationsWithTimeout:2 handler:nil];

    // see that it only ran once
    XCTAssertEqual(counter, 1);
}

- (void)testInvalidation
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"Work item called"];
    XCTestExpectation * cancelExpectation = [self expectationWithDescription:@"Work item canceled"];

    MTRAsyncWorkQueue * workQueue = [[MTRAsyncWorkQueue alloc] initWithContext:NSNull.null];

    MTRAsyncWorkItem * workItem1 = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    workItem1.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        // Give the code enqueing the other items a chance to run, so they can
        // actually get canceled.
        sleep(1);
        [workQueue invalidate];
        XCTAssertFalse(completion(MTRAsyncWorkComplete));
        [expectation fulfill];
    };
    // No cancel handler on purpose.
    [workQueue enqueueWorkItem:workItem1];

    MTRAsyncWorkItem * workItem2 = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    workItem2.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        XCTFail("Should not have been called");
        completion(MTRAsyncWorkComplete);
    };
    workItem2.cancelHandler = nil;
    [workQueue enqueueWorkItem:workItem2];

    MTRAsyncWorkItem * workItem3 = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    workItem3.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        XCTFail("Should not have been called");
        completion(MTRAsyncWorkComplete);
    };
    workItem3.cancelHandler = ^{
        [cancelExpectation fulfill];
    };
    [workQueue enqueueWorkItem:workItem3];

    [self waitForExpectations:@[ expectation, cancelExpectation ] timeout:2];
}

- (void)testBatching
{
    XCTestExpectation * workItem1ReadyExpectation = [self expectationWithDescription:@"Work item 1 called"];
    __block BOOL workItem2BatchingCalled = NO;
    __block BOOL workItem2ReadyCalled = NO;
    XCTestExpectation * workItem3ReadyExpectation = [self expectationWithDescription:@"Work item 3 called"];

    MTRAsyncWorkQueue * workQueue = [[MTRAsyncWorkQueue alloc] initWithContext:@42];

    MTRAsyncWorkItem * workItem0 = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    workItem0.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        // While processing item 0, enqueue additional items to test batching
        MTRAsyncWorkItem * workItem1 = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
        [workItem1 setReadyHandler:^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
            [workItem1ReadyExpectation fulfill];
            completion(MTRAsyncWorkComplete);
        }];
        [workItem1 setBatchingID:1
                            data:@(1)
                         handler:^(id _Nonnull opaqueDataFirst, id _Nonnull opaqueDataSecond) {
                             XCTAssertEqualObjects(opaqueDataFirst, @(1));
                             XCTAssertEqualObjects(opaqueDataSecond, @(2));
                             return MTRBatchedFully;
                         }];
        [workQueue enqueueWorkItem:workItem1 description:@"workItem1"];

        MTRAsyncWorkItem * workItem2 = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
        [workItem2 setReadyHandler:^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
            workItem2ReadyCalled = YES;
            completion(MTRAsyncWorkComplete);
        }];
        [workItem2 setBatchingID:1
                            data:@(2)
                         handler:^(id _Nonnull opaqueDataFirst, id _Nonnull opaqueDataSecond) {
                             workItem2BatchingCalled = YES;
                             return MTRBatchedPartially;
                         }];
        [workQueue enqueueWorkItem:workItem2 description:@"workItem2"];

        MTRAsyncWorkItem * workItem3 = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
        [workItem3 setReadyHandler:^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
            [workItem3ReadyExpectation fulfill];
            completion(MTRAsyncWorkComplete);
        }];
        [workQueue enqueueWorkItem:workItem3 description:@"workItem3"];

        completion(MTRAsyncWorkComplete);
    };
    [workQueue enqueueWorkItem:workItem0];

    [self waitForExpectations:@[ workItem1ReadyExpectation, workItem3ReadyExpectation ] timeout:3];

    XCTAssertFalse(workItem2BatchingCalled);
    XCTAssertFalse(workItem2ReadyCalled);
}

- (void)testDuplicate
{
    XCTestExpectation * workItem0ReadyExpectation = [self expectationWithDescription:@"Work item 0 called"];
    XCTestExpectation * workItem6ReadyExpectation = [self expectationWithDescription:@"Work item 6 called"];

    MTRAsyncWorkQueue * workQueue = [[MTRAsyncWorkQueue alloc] initWithContext:NSNull.null];

    MTRAsyncWorkItem * workItem0 = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    workItem0.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        // While processing item 0, enqueue additional items to test duplicate checking
        MTRAsyncWorkItem * workItem1 = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
        workItem1.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
            completion(MTRAsyncWorkComplete);
        };
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

        MTRAsyncWorkItem * workItem2 = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
        workItem2.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
            completion(MTRAsyncWorkComplete);
        };
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

        MTRAsyncWorkItem * workItem3 = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
        workItem3.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
            completion(MTRAsyncWorkComplete);
        };
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
        XCTAssertTrue([workQueue hasDuplicateForTypeID:1 workItemData:@(1)]);
        XCTAssertTrue([workQueue hasDuplicateForTypeID:1 workItemData:@(2)]);
        XCTAssertTrue([workQueue hasDuplicateForTypeID:2 workItemData:@(1)]);

        XCTAssertFalse([workQueue hasDuplicateForTypeID:0 workItemData:@(1)]);
        XCTAssertFalse([workQueue hasDuplicateForTypeID:0 workItemData:@(2)]);
        XCTAssertFalse([workQueue hasDuplicateForTypeID:1 workItemData:@(0)]);
        XCTAssertFalse([workQueue hasDuplicateForTypeID:1 workItemData:@(3)]);
        XCTAssertFalse([workQueue hasDuplicateForTypeID:2 workItemData:@(2)]);

        // Test returning *isDuplicate=NO and queuing one extra duplicate item, and that the extra item runs

        // First have a regular item with ID/data == 3/1
        MTRAsyncWorkItem * workItem4 = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
        workItem4.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
            completion(MTRAsyncWorkComplete);
        };
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

        XCTAssertTrue([workQueue hasDuplicateForTypeID:3 workItemData:@(1)]);

        // Have a barrier item with ID/data == 3/1 that returns *isDuplicate=NO
        MTRAsyncWorkItem * workItem5 = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
        workItem5.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
            completion(MTRAsyncWorkComplete);
        };
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
        XCTAssertFalse([workQueue hasDuplicateForTypeID:3 workItemData:@(1)]);

        // Now add regular regular item with ID/data == 3/1
        MTRAsyncWorkItem * workItem6 = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
        workItem6.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
            completion(MTRAsyncWorkComplete);
            [workItem6ReadyExpectation fulfill];
        };
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
        XCTAssertTrue([workQueue hasDuplicateForTypeID:3 workItemData:@(1)]);

        completion(MTRAsyncWorkComplete);
        [workItem0ReadyExpectation fulfill];
    };
    [workQueue enqueueWorkItem:workItem0];

    [self waitForExpectations:@[ workItem0ReadyExpectation, workItem6ReadyExpectation ] timeout:5];
}

- (void)testNoRetainCycles
{
    NSPointerArray * objects = [NSPointerArray weakObjectsPointerArray];
    @autoreleasepool {
        __block MTRAsyncWorkCompletionBlock workCompletion;
        @autoreleasepool {
            MTRAsyncWorkQueue * queue = [[MTRAsyncWorkQueue alloc] initWithContext:self];
            [objects addPointer:(__bridge void *) queue];
            MTRAsyncWorkItem * work = [[MTRAsyncWorkItem alloc] initWithQueue:self.backgroundQueue];
            dispatch_semaphore_t workReady = dispatch_semaphore_create(0);
            dispatch_semaphore_t readyHandlerDone = dispatch_semaphore_create(0);
            work.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
                dispatch_semaphore_wait(workReady, DISPATCH_TIME_FOREVER);
                workCompletion = completion; // retain outside inner ARP, simulating ongoing work
                dispatch_async(self.backgroundQueue, ^{
                    dispatch_semaphore_signal(readyHandlerDone);
                });
            };
            [objects addPointer:(__bridge void *) work];
            [queue enqueueWorkItem:work];
            XCTAssertEqual(objects.allObjects.count, 2);

            dispatch_semaphore_signal(workReady);
            dispatch_semaphore_wait(readyHandlerDone, DISPATCH_TIME_FOREVER);
            // Not calling [queue invalidate]
        }

        // We've dropped the work queue but the work item may
        // still be retained by the work completion block.
        XCTAssertNotNil(workCompletion);
        NSArray * retained = objects.allObjects;
        if (retained.count > 0) {
            XCTAssertEqual(retained.count, 1);
            XCTAssert([retained.firstObject isKindOfClass:MTRAsyncWorkItem.class]);
        }
        workCompletion = nil;
    }

    // Everything should be gone now
    XCTAssertEqualObjects(objects.allObjects, @[]);
}

- (void)testContextLoss
{
    // Use a CFTypeRef so we can explicitly control the reference count
    CFTypeRef testContext = CFBridgingRetain([[NSObject alloc] init]);
    XCTAssertEqual(CFGetRetainCount(testContext), 1, @"internal test error");
    MTRAsyncWorkQueue * queue = [[MTRAsyncWorkQueue alloc] initWithContext:(__bridge id) testContext];

    dispatch_semaphore_t ready = dispatch_semaphore_create(0);
    dispatch_semaphore_t proceed = dispatch_semaphore_create(0);
    MTRAsyncWorkItem * setup = [[MTRAsyncWorkItem alloc] initWithQueue:self.backgroundQueue];
    setup.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        // dispatch again because `context` is retained by the caller
        dispatch_async(self.backgroundQueue, ^{
            dispatch_semaphore_signal(ready);
            dispatch_semaphore_wait(proceed, DISPATCH_TIME_FOREVER);
            completion(MTRAsyncWorkComplete);
        });
    };
    [queue enqueueWorkItem:setup];

    // Enqueue work item. It should not run because we will drop the context first.
    XCTestExpectation * workRun = [self expectationWithDescription:@"work ready handler run"];
    workRun.inverted = YES;
    MTRAsyncWorkItem * work = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    work.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        [workRun fulfill];
    };
    [queue enqueueWorkItem:work];

    dispatch_semaphore_wait(ready, DISPATCH_TIME_FOREVER);
    XCTAssertEqual(CFGetRetainCount(testContext), 1, @"internal test error");
    CFRelease(testContext);
    dispatch_semaphore_signal(proceed);
    [self waitForExpectationsWithTimeout:1 handler:nil];
}

- (void)testItemsConcurrently
{
    MTRAsyncWorkQueue * workQueue = [[MTRAsyncWorkQueue alloc] initWithContext:NSNull.null width:3];

    XCTestExpectation * first3WorkItemsExecutedExpectation = [self expectationWithDescription:@"First 3 work items executed"];
    XCTestExpectation * first3WorkItemsSleptExpectation = [self expectationWithDescription:@"First 3 work items slept"];
    __block os_unfair_lock counterLock = OS_UNFAIR_LOCK_INIT;
    __block int beforeSleepCounter = 0;
    __block int afterSleepCounter = 0;
    __auto_type sleep1ReadyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        os_unfair_lock_lock(&counterLock);
        beforeSleepCounter++;
        if (beforeSleepCounter == 3) {
            [first3WorkItemsExecutedExpectation fulfill];
        }
        os_unfair_lock_unlock(&counterLock);
        sleep(1);
        os_unfair_lock_lock(&counterLock);
        afterSleepCounter++;
        if (afterSleepCounter == 3) {
            [first3WorkItemsSleptExpectation fulfill];
        }
        os_unfair_lock_unlock(&counterLock);
        completion(MTRAsyncWorkComplete);
    };

    MTRAsyncWorkItem * workItem1 = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    workItem1.readyHandler = sleep1ReadyHandler;
    [workQueue enqueueWorkItem:workItem1 descriptionWithFormat:@"work item %d", 1];

    MTRAsyncWorkItem * workItem2 = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    workItem2.readyHandler = sleep1ReadyHandler;
    [workQueue enqueueWorkItem:workItem2 descriptionWithFormat:@"work item %d", 2];

    MTRAsyncWorkItem * workItem3 = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    workItem3.readyHandler = sleep1ReadyHandler;
    [workQueue enqueueWorkItem:workItem3 descriptionWithFormat:@"work item %d", 3];

    // This is the item after the first 3, and should only execute when one of them finished
    XCTestExpectation * lastWorkItemWaitedExpectation = [self expectationWithDescription:@"Last work item waited properly"];
    MTRAsyncWorkItem * workItemLast = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    workItemLast.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        // expect this to have waited until at least one of the above items finished after sleep() and incremented counter
        os_unfair_lock_lock(&counterLock);
        XCTAssert(afterSleepCounter > 0);
        [lastWorkItemWaitedExpectation fulfill];
        os_unfair_lock_unlock(&counterLock);
        completion(MTRAsyncWorkComplete);
    };
    [workQueue enqueueWorkItem:workItemLast description:@"last work item"];

    [self waitForExpectations:@[ first3WorkItemsExecutedExpectation ] timeout:2];
    // the before-sleep counter should have reached 3 immediately as they all run concurrently.
    os_unfair_lock_lock(&counterLock);
    XCTAssertEqual(afterSleepCounter, 0);
    os_unfair_lock_unlock(&counterLock);

    [self waitForExpectations:@[ lastWorkItemWaitedExpectation, first3WorkItemsSleptExpectation ] timeout:2];

    // see that all 3 first items ran and slept
    os_unfair_lock_lock(&counterLock);
    XCTAssertEqual(beforeSleepCounter, 3);
    XCTAssertEqual(afterSleepCounter, 3);
    os_unfair_lock_unlock(&counterLock);
}

- (void)testBatchingWithConcurrentWidth
{
    // This test exposes a bug at MTRAsyncWorkQueue.mm:407 where [_items removeObjectAtIndex:1]
    // is hardcoded instead of using firstNonRunningItemIndex.
    //
    // Bug scenario with width=3:
    // - Items A, B, C fill capacity (all running at indices 0, 1, 2)
    // - Items D, E are queued (at indices 3, 4)
    // - Item A completes and is removed
    // - Items array becomes: [B_running, C_running, D, E] (indices 0, 1, 2, 3)
    // - Start D at index 2: _runningWorkItemCount becomes 3
    // - Try to batch E at index 3 into D
    // - Bug: removes index 1 (C!) instead of index 3 (E)
    // - Result: C is removed but still running, causing assertion failure when it completes

    MTRAsyncWorkQueue * workQueue = [[MTRAsyncWorkQueue alloc] initWithContext:@"test" width:3];

    XCTestExpectation * blockingItem0Started = [self expectationWithDescription:@"Blocking item 0 started"];
    XCTestExpectation * blockingItem1Started = [self expectationWithDescription:@"Blocking item 1 started"];
    XCTestExpectation * blockingItem2Started = [self expectationWithDescription:@"Blocking item 2 started"];
    XCTestExpectation * batchingItemExecuted = [self expectationWithDescription:@"Batching item executed"];
    XCTestExpectation * batchedItemShouldNotExecute = [self expectationWithDescription:@"Batched item should not execute"];
    batchedItemShouldNotExecute.inverted = YES;

    NSArray<XCTestExpectation *> * blockingStartedExpectations = @[ blockingItem0Started, blockingItem1Started, blockingItem2Started ];
    dispatch_semaphore_t blockingItemsCanFinish = dispatch_semaphore_create(0);

    // Enqueue 3 blocking items to fill the width=3 capacity
    for (int i = 0; i < 3; i++) {
        XCTestExpectation * startedExpectation = blockingStartedExpectations[i];
        MTRAsyncWorkItem * blockingItem = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
        blockingItem.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
            [startedExpectation fulfill];
            dispatch_semaphore_wait(blockingItemsCanFinish, DISPATCH_TIME_FOREVER);
            completion(MTRAsyncWorkComplete);
        };
        [workQueue enqueueWorkItem:blockingItem descriptionWithFormat:@"blocking item %d", i];
    }

    // Enqueue 2 batchable items - these will be queued because capacity is full
    NSMutableArray * mergedData = [NSMutableArray arrayWithObject:@"D"];

    MTRAsyncWorkItem * batchingItem = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    batchingItem.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        // Should execute with merged data from both D and E
        XCTAssertEqual(mergedData.count, 2, @"Item E should have been batched into item D");
        XCTAssertEqualObjects(mergedData[0], @"D");
        XCTAssertEqualObjects(mergedData[1], @"E");
        completion(MTRAsyncWorkComplete);
        [batchingItemExecuted fulfill];
    };
    [batchingItem setBatchingID:100
                           data:mergedData
                        handler:^(id first, id second) {
                            NSMutableArray * firstArray = (NSMutableArray *) first;
                            NSArray * secondArray = (NSArray *) second;
                            [firstArray addObjectsFromArray:secondArray];
                            return MTRBatchedFully;
                        }];
    [workQueue enqueueWorkItem:batchingItem description:@"batching item D"];

    MTRAsyncWorkItem * batchedItem = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    batchedItem.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        // Should NOT execute - should be batched into item D
        completion(MTRAsyncWorkComplete);
        [batchedItemShouldNotExecute fulfill];
    };
    [batchedItem setBatchingID:100
                          data:@[ @"E" ]
                       handler:^(id first, id second) {
                           return MTRNotBatched;
                       }];
    [workQueue enqueueWorkItem:batchedItem description:@"batchable item E"];

    // Wait for all blocking items to start
    [self waitForExpectations:@[ blockingItem0Started, blockingItem1Started, blockingItem2Started ] timeout:2];

    // Release ONE blocking item to trigger batching
    // When it completes, item D will start and should batch item E
    // At this point: [blockingB_running, blockingC_running, D, E]
    // D starts at index 2, E is at index 3, but bug removes index 1!
    dispatch_semaphore_signal(blockingItemsCanFinish);

    // Wait for batching to complete
    [self waitForExpectations:@[ batchingItemExecuted, batchedItemShouldNotExecute ] timeout:2];

    // Release remaining blocking items
    dispatch_semaphore_signal(blockingItemsCanFinish);
    dispatch_semaphore_signal(blockingItemsCanFinish);

    // Add semaphores for final items synchronization
    dispatch_semaphore_t finalItemsStarted = dispatch_semaphore_create(0);
    dispatch_semaphore_t finalItemsCanFinish = dispatch_semaphore_create(0);

    // Enqueue final items to ensure queue drains properly
    XCTestExpectation * finalItem0 = [self expectationWithDescription:@"Final item 0"];
    XCTestExpectation * finalItem1 = [self expectationWithDescription:@"Final item 1"];
    XCTestExpectation * finalItem2 = [self expectationWithDescription:@"Final item 2"];
    NSArray<XCTestExpectation *> * finalExpectations = @[ finalItem0, finalItem1, finalItem2 ];

    for (int i = 0; i < 3; i++) {
        XCTestExpectation * expectation = finalExpectations[i];
        MTRAsyncWorkItem * finalItem = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
        finalItem.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
            // Signal that this item started, then wait to finish
            dispatch_semaphore_signal(finalItemsStarted);
            dispatch_semaphore_wait(finalItemsCanFinish, DISPATCH_TIME_FOREVER);

            completion(MTRAsyncWorkComplete);
            [expectation fulfill];
        };
        [workQueue enqueueWorkItem:finalItem descriptionWithFormat:@"final item %d", i];
    }

    // Wait for all 3 final items to start (proving queue recovered from batching)
    dispatch_semaphore_wait(finalItemsStarted, dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC));
    dispatch_semaphore_wait(finalItemsStarted, dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC));
    dispatch_semaphore_wait(finalItemsStarted, dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC));

    // Now let them all complete
    dispatch_semaphore_signal(finalItemsCanFinish);
    dispatch_semaphore_signal(finalItemsCanFinish);
    dispatch_semaphore_signal(finalItemsCanFinish);

    [self waitForExpectations:finalExpectations timeout:2];

    // Verify queue is properly drained
    XCTAssertEqual([workQueue itemCount], 0, @"Queue should be empty after all items complete");
}

#pragma mark - User commands ahead of unsent reads

// A read that has sent nothing is queued, not running, so a user command ("jumper") may go ahead of it.
- (MTRAsyncWorkItem *)_readItemNamed:(NSString *)name order:(NSMutableArray<NSString *> *)order sendNow:(BOOL)sendNow onQueue:(MTRAsyncWorkQueue *)workQueue
{
    MTRAsyncWorkItem * item = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    item.tracksSend = YES;
    MTRAsyncWorkItem * __weak weakItem = item;
    item.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        @synchronized(order) {
            [order addObject:name];
        }
        if (sendNow) {
            [workQueue workItem:weakItem readyToSend:^ {}];
        }
        completion(MTRAsyncWorkComplete);
    };
    return item;
}

- (MTRAsyncWorkItem *)_jumperNamed:(NSString *)name order:(NSMutableArray<NSString *> *)order
{
    MTRAsyncWorkItem * item = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    item.jumpsUnsentWork = YES;
    item.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        @synchronized(order) {
            [order addObject:name];
        }
        completion(MTRAsyncWorkComplete);
    };
    return item;
}

// Holds the queue's only slot until the returned block is called.
- (dispatch_block_t)_holdQueue:(MTRAsyncWorkQueue *)workQueue order:(NSMutableArray<NSString *> *)order
{
    dispatch_semaphore_t started = dispatch_semaphore_create(0);
    dispatch_semaphore_t release = dispatch_semaphore_create(0);
    MTRAsyncWorkItem * hold = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    hold.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        @synchronized(order) {
            [order addObject:@"hold"];
        }
        dispatch_semaphore_signal(started);
        dispatch_semaphore_wait(release, DISPATCH_TIME_FOREVER);
        completion(MTRAsyncWorkComplete);
    };
    [workQueue enqueueWorkItem:hold description:@"hold"];
    XCTAssertEqual(dispatch_semaphore_wait(started, dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC)), 0);
    return ^{
        dispatch_semaphore_signal(release);
    };
}

- (void)_waitForOrderCount:(NSUInteger)count order:(NSMutableArray<NSString *> *)order
{
    XCTNSPredicateExpectation * done = [[XCTNSPredicateExpectation alloc] initWithPredicate:[NSPredicate predicateWithBlock:^BOOL(id _Nullable object, NSDictionary * _Nullable bindings) {
        @synchronized(order) {
            return order.count >= count;
        }
    }]
                                                                                     object:nil];
    [self waitForExpectations:@[ done ] timeout:2];
}

- (void)testJumperRunsAheadOfQueuedReads
{
    MTRAsyncWorkQueue * workQueue = [[MTRAsyncWorkQueue alloc] initWithContext:NSNull.null];
    NSMutableArray<NSString *> * order = [NSMutableArray array];
    dispatch_block_t release = [self _holdQueue:workQueue order:order];
    [workQueue enqueueWorkItem:[self _readItemNamed:@"A" order:order sendNow:YES onQueue:workQueue] description:@"read A"];
    [workQueue enqueueWorkItem:[self _readItemNamed:@"B" order:order sendNow:YES onQueue:workQueue] description:@"read B"];
    [workQueue enqueueWorkItem:[self _jumperNamed:@"J" order:order] description:@"jumper J"];
    release();
    [self _waitForOrderCount:4 order:order];
    @synchronized(order) {
        XCTAssertEqualObjects(order, (@[ @"hold", @"J", @"A", @"B" ]));
    }
}

- (void)testJumperRunsBesideUnsentRead
{
    MTRAsyncWorkQueue * workQueue = [[MTRAsyncWorkQueue alloc] initWithContext:NSNull.null];
    NSMutableArray<NSString *> * order = [NSMutableArray array];
    dispatch_semaphore_t readRunning = dispatch_semaphore_create(0);
    __block MTRAsyncWorkCompletionBlock readCompletion;
    MTRAsyncWorkItem * read = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    read.tracksSend = YES;
    read.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        readCompletion = completion; // phase 1: waiting for a session, nothing sent
        dispatch_semaphore_signal(readRunning);
    };
    [workQueue enqueueWorkItem:read description:@"unsent read R"];
    XCTAssertEqual(dispatch_semaphore_wait(readRunning, dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC)), 0);

    [workQueue enqueueWorkItem:[self _jumperNamed:@"J" order:order] description:@"jumper J"];
    [self _waitForOrderCount:1 order:order];
    @synchronized(order) {
        XCTAssertEqualObjects(order, (@[ @"J" ]), @"the jumper starts while the only slot holds an unsent read");
    }
    readCompletion(MTRAsyncWorkComplete);
}

- (void)testOnlyOneJumperAboveWidth
{
    MTRAsyncWorkQueue * workQueue = [[MTRAsyncWorkQueue alloc] initWithContext:NSNull.null];
    dispatch_semaphore_t readRunning = dispatch_semaphore_create(0);
    __block MTRAsyncWorkCompletionBlock readCompletion;
    MTRAsyncWorkItem * read = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    read.tracksSend = YES;
    read.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        readCompletion = completion;
        dispatch_semaphore_signal(readRunning);
    };
    [workQueue enqueueWorkItem:read description:@"unsent read R"];
    XCTAssertEqual(dispatch_semaphore_wait(readRunning, dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC)), 0);

    // Semaphores, not expectations: on a queue without jumpers these items run after the test has returned.
    os_unfair_lock lock = OS_UNFAIR_LOCK_INIT;
    os_unfair_lock * lockPtr = &lock;
    __block int started = 0;
    dispatch_semaphore_t jumperStarted = dispatch_semaphore_create(0);
    dispatch_semaphore_t j1Release = dispatch_semaphore_create(0);
    for (int i = 0; i < 2; i++) {
        MTRAsyncWorkItem * jumper = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
        jumper.jumpsUnsentWork = YES;
        jumper.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
            os_unfair_lock_lock(lockPtr);
            BOOL first = (++started == 1);
            os_unfair_lock_unlock(lockPtr);
            dispatch_semaphore_signal(jumperStarted);
            if (first) {
                dispatch_semaphore_wait(j1Release, dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC));
            }
            completion(MTRAsyncWorkComplete);
        };
        [workQueue enqueueWorkItem:jumper descriptionWithFormat:@"jumper J%d", i + 1];
    }
    XCTAssertEqual(dispatch_semaphore_wait(jumperStarted, dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC)), 0, @"one jumper runs above width");
    // Give the second jumper every chance to start, too.
    XCTAssertNotEqual(dispatch_semaphore_wait(jumperStarted, dispatch_time(DISPATCH_TIME_NOW, (int64_t) (0.5 * NSEC_PER_SEC))), 0, @"only one jumper above width");
    dispatch_semaphore_signal(j1Release);
    XCTAssertEqual(dispatch_semaphore_wait(jumperStarted, dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC)), 0, @"the second jumper runs once the first completes");
    os_unfair_lock_lock(lockPtr);
    XCTAssertEqual(started, 2);
    os_unfair_lock_unlock(lockPtr);
    readCompletion(MTRAsyncWorkComplete);
}

- (void)testOnlyOneSendTrackingJumperAboveWidth
{
    // The cap on user commands above the width holds even for user commands that also track their send: while one
    // runs beside work that has sent nothing, a second waits for it, although everything running is still unsent.
    MTRAsyncWorkQueue * workQueue = [[MTRAsyncWorkQueue alloc] initWithContext:NSNull.null];
    dispatch_semaphore_t readRunning = dispatch_semaphore_create(0);
    __block MTRAsyncWorkCompletionBlock readCompletion;
    MTRAsyncWorkItem * read = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    read.tracksSend = YES;
    read.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        readCompletion = completion;
        dispatch_semaphore_signal(readRunning);
    };
    [workQueue enqueueWorkItem:read description:@"unsent read R"];
    XCTAssertEqual(dispatch_semaphore_wait(readRunning, dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC)), 0);

    // Semaphores, not expectations: a jumper that never starts must not fulfill anything after the test returns.
    os_unfair_lock lock = OS_UNFAIR_LOCK_INIT;
    os_unfair_lock * lockPtr = &lock;
    __block int started = 0;
    dispatch_semaphore_t jumperStarted = dispatch_semaphore_create(0);
    dispatch_semaphore_t j1Release = dispatch_semaphore_create(0);
    for (int i = 0; i < 2; i++) {
        MTRAsyncWorkItem * jumper = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
        jumper.jumpsUnsentWork = YES;
        jumper.tracksSend = YES; // and it never calls workItem:readyToSend:, so it stays unsent
        jumper.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
            os_unfair_lock_lock(lockPtr);
            BOOL first = (++started == 1);
            os_unfair_lock_unlock(lockPtr);
            dispatch_semaphore_signal(jumperStarted);
            if (first) {
                dispatch_semaphore_wait(j1Release, dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC));
            }
            completion(MTRAsyncWorkComplete);
        };
        [workQueue enqueueWorkItem:jumper descriptionWithFormat:@"send-tracking jumper J%d", i + 1];
    }
    XCTAssertEqual(dispatch_semaphore_wait(jumperStarted, dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC)), 0, @"one jumper runs above width");
    XCTAssertNotEqual(dispatch_semaphore_wait(jumperStarted, dispatch_time(DISPATCH_TIME_NOW, (int64_t) (0.5 * NSEC_PER_SEC))), 0, @"a second send-tracking jumper does not start while the first runs");
    dispatch_semaphore_signal(j1Release);
    XCTAssertEqual(dispatch_semaphore_wait(jumperStarted, dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC)), 0, @"the second jumper runs once the first completes");
    os_unfair_lock_lock(lockPtr);
    XCTAssertEqual(started, 2);
    os_unfair_lock_unlock(lockPtr);
    readCompletion(MTRAsyncWorkComplete);
}

- (void)testReadSendAfterJumperSendReturns
{
    MTRAsyncWorkQueue * workQueue = [[MTRAsyncWorkQueue alloc] initWithContext:NSNull.null];
    dispatch_semaphore_t readRunning = dispatch_semaphore_create(0);
    __block MTRAsyncWorkItem * __weak weakRead;
    __block MTRAsyncWorkCompletionBlock readCompletion;
    MTRAsyncWorkItem * read = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    read.tracksSend = YES;
    weakRead = read;
    read.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        readCompletion = completion;
        dispatch_semaphore_signal(readRunning);
    };
    [workQueue enqueueWorkItem:read description:@"unsent read R"];
    XCTAssertEqual(dispatch_semaphore_wait(readRunning, dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC)), 0);

    os_unfair_lock lock = OS_UNFAIR_LOCK_INIT;
    os_unfair_lock * lockPtr = &lock;
    NSMutableArray<NSString *> * events = [NSMutableArray array];
    dispatch_semaphore_t jumperSending = dispatch_semaphore_create(0);
    dispatch_semaphore_t jumperSendRelease = dispatch_semaphore_create(0);
    MTRAsyncWorkItem * jumper = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    jumper.jumpsUnsentWork = YES;
    MTRAsyncWorkItem * __weak weakJumper = jumper;
    jumper.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        [workQueue workItem:weakJumper readyToSend:^{
            dispatch_semaphore_signal(jumperSending);
            dispatch_semaphore_wait(jumperSendRelease, DISPATCH_TIME_FOREVER);
            os_unfair_lock_lock(lockPtr);
            [events addObject:@"J sent"];
            os_unfair_lock_unlock(lockPtr);
        }];
        completion(MTRAsyncWorkComplete);
    };
    [workQueue enqueueWorkItem:jumper description:@"jumper J"];
    XCTAssertEqual(dispatch_semaphore_wait(jumperSending, dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC)), 0, @"jumper reached its send");

    XCTestExpectation * readSent = [self expectationWithDescription:@"read sent"];
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0), ^{
        [workQueue workItem:weakRead readyToSend:^{
            os_unfair_lock_lock(lockPtr);
            [events addObject:@"R sent"];
            os_unfair_lock_unlock(lockPtr);
            [readSent fulfill];
        }];
    });
    [NSThread sleepForTimeInterval:0.3];
    dispatch_semaphore_signal(jumperSendRelease);
    [self waitForExpectations:@[ readSent ] timeout:2];
    os_unfair_lock_lock(lockPtr);
    XCTAssertEqualObjects(events, (@[ @"J sent", @"R sent" ]), @"the read sends after the jumper's send returns");
    os_unfair_lock_unlock(lockPtr);
    readCompletion(MTRAsyncWorkComplete);
}

- (void)testJumperInPhase1DoesNotHoldReadSend
{
    MTRAsyncWorkQueue * workQueue = [[MTRAsyncWorkQueue alloc] initWithContext:NSNull.null];
    dispatch_semaphore_t readRunning = dispatch_semaphore_create(0);
    __block MTRAsyncWorkItem * __weak weakRead;
    __block MTRAsyncWorkCompletionBlock readCompletion;
    MTRAsyncWorkItem * read = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    read.tracksSend = YES;
    weakRead = read;
    read.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        readCompletion = completion;
        dispatch_semaphore_signal(readRunning);
    };
    [workQueue enqueueWorkItem:read description:@"unsent read R"];
    XCTAssertEqual(dispatch_semaphore_wait(readRunning, dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC)), 0);

    __block MTRAsyncWorkCompletionBlock jumperCompletion;
    dispatch_semaphore_t jumperRunning = dispatch_semaphore_create(0);
    MTRAsyncWorkItem * jumper = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    jumper.jumpsUnsentWork = YES;
    jumper.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        jumperCompletion = completion; // stuck in phase 1: never calls readyToSend
        dispatch_semaphore_signal(jumperRunning);
    };
    [workQueue enqueueWorkItem:jumper description:@"jumper J"];
    // At the base the jumper waits behind R; either way R's send must not wait for it.
    dispatch_semaphore_wait(jumperRunning, dispatch_time(DISPATCH_TIME_NOW, (int64_t) (0.5 * NSEC_PER_SEC)));

    __block BOOL sentInsideCall = NO;
    __block BOOL inCall = YES;
    [workQueue workItem:weakRead readyToSend:^{
        sentInsideCall = inCall;
    }];
    inCall = NO;
    XCTAssertTrue(sentInsideCall, @"a jumper in phase 1 does not hold back a read that has its session");
    readCompletion(MTRAsyncWorkComplete);
    // The jumper has run by now (beside R, or after it); let it finish so the queue drains.
    if (!jumperCompletion) {
        dispatch_semaphore_wait(jumperRunning, dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC));
    }
    if (jumperCompletion) {
        jumperCompletion(MTRAsyncWorkComplete);
    }
}

- (void)testReadPhasesBackToBackWithoutJumper
{
    MTRAsyncWorkQueue * workQueue = [[MTRAsyncWorkQueue alloc] initWithContext:NSNull.null];
    XCTestExpectation * done = [self expectationWithDescription:@"read done"];
    __block BOOL sentInsideCall = NO;
    MTRAsyncWorkItem * read = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    read.tracksSend = YES;
    MTRAsyncWorkItem * __weak weakRead = read;
    read.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        __block BOOL inCall = YES;
        [workQueue workItem:weakRead readyToSend:^{
            sentInsideCall = inCall;
        }];
        inCall = NO;
        completion(MTRAsyncWorkComplete);
        [done fulfill];
    };
    [workQueue enqueueWorkItem:read description:@"read R"];
    [self waitForExpectations:@[ done ] timeout:2];
    XCTAssertTrue(sentInsideCall, @"with no jumper, a read's send runs inside its readyToSend call");
}

- (void)testJumperWaitsForSentRead
{
    MTRAsyncWorkQueue * workQueue = [[MTRAsyncWorkQueue alloc] initWithContext:NSNull.null];
    NSMutableArray<NSString *> * order = [NSMutableArray array];
    dispatch_semaphore_t readSent = dispatch_semaphore_create(0);
    dispatch_semaphore_t readRelease = dispatch_semaphore_create(0);
    MTRAsyncWorkItem * read = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    read.tracksSend = YES;
    MTRAsyncWorkItem * __weak weakRead = read;
    read.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        [workQueue workItem:weakRead readyToSend:^ {}];
        dispatch_semaphore_signal(readSent);
        dispatch_semaphore_wait(readRelease, DISPATCH_TIME_FOREVER);
        @synchronized(order) {
            [order addObject:@"R"];
        }
        completion(MTRAsyncWorkComplete);
    };
    [workQueue enqueueWorkItem:read description:@"sent read R"];
    XCTAssertEqual(dispatch_semaphore_wait(readSent, dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC)), 0);
    [workQueue enqueueWorkItem:[self _jumperNamed:@"J" order:order] description:@"jumper J"];
    [NSThread sleepForTimeInterval:0.3];
    @synchronized(order) {
        XCTAssertEqual(order.count, 0, @"a read that has sent finishes first");
    }
    dispatch_semaphore_signal(readRelease);
    [self _waitForOrderCount:2 order:order];
    @synchronized(order) {
        XCTAssertEqualObjects(order, (@[ @"R", @"J" ]));
    }
}

- (void)testCancelQueuedItemNeverRuns
{
    MTRAsyncWorkQueue * workQueue = [[MTRAsyncWorkQueue alloc] initWithContext:NSNull.null];
    NSMutableArray<NSString *> * order = [NSMutableArray array];
    dispatch_block_t release = [self _holdQueue:workQueue order:order];
    __block int readyCalls = 0;
    XCTestExpectation * cancelled = [self expectationWithDescription:@"cancel handler ran"];
    MTRAsyncWorkItem * victim = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    victim.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        readyCalls++;
        completion(MTRAsyncWorkComplete);
    };
    victim.cancelHandler = ^{
        [cancelled fulfill];
    };
    [workQueue enqueueWorkItem:victim description:@"victim"];
    [workQueue enqueueWorkItem:[self _jumperNamed:@"next" order:order] description:@"next"];
    XCTAssertTrue([workQueue cancelWorkItemIfUnsent:victim]);
    release();
    [self waitForExpectations:@[ cancelled ] timeout:2];
    [self _waitForOrderCount:2 order:order];
    [NSThread sleepForTimeInterval:0.2];
    XCTAssertEqual(readyCalls, 0);
    XCTAssertEqual([workQueue itemCount], 0);
}

- (void)testCancelSentItemRefused
{
    MTRAsyncWorkQueue * workQueue = [[MTRAsyncWorkQueue alloc] initWithContext:NSNull.null];
    dispatch_semaphore_t sent = dispatch_semaphore_create(0);
    dispatch_semaphore_t release = dispatch_semaphore_create(0);
    XCTestExpectation * completed = [self expectationWithDescription:@"sent item completed"];
    MTRAsyncWorkItem * item = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    item.jumpsUnsentWork = YES;
    MTRAsyncWorkItem * __weak weakItem = item;
    item.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        [workQueue workItem:weakItem readyToSend:^ {}];
        dispatch_semaphore_signal(sent);
        dispatch_semaphore_wait(release, DISPATCH_TIME_FOREVER);
        XCTAssertTrue(completion(MTRAsyncWorkComplete));
        [completed fulfill];
    };
    [workQueue enqueueWorkItem:item description:@"sent item"];
    XCTAssertEqual(dispatch_semaphore_wait(sent, dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC)), 0);
    XCTAssertFalse([workQueue cancelWorkItemIfUnsent:item]);
    dispatch_semaphore_signal(release);
    [self waitForExpectations:@[ completed ] timeout:2];
}

- (void)testInvalidateWithJumperAboveWidthFreesItsSlot
{
    MTRAsyncWorkQueue * workQueue = [[MTRAsyncWorkQueue alloc] initWithContext:NSNull.null];
    dispatch_semaphore_t readRunning = dispatch_semaphore_create(0);
    MTRAsyncWorkItem * read = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    read.tracksSend = YES;
    read.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        dispatch_semaphore_signal(readRunning); // never completes before invalidate
    };
    [workQueue enqueueWorkItem:read description:@"unsent read R"];
    XCTAssertEqual(dispatch_semaphore_wait(readRunning, dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC)), 0);

    dispatch_semaphore_t jumperRunning = dispatch_semaphore_create(0);
    MTRAsyncWorkItem * jumper = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    jumper.jumpsUnsentWork = YES;
    jumper.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        dispatch_semaphore_signal(jumperRunning); // never completes before invalidate
    };
    [workQueue enqueueWorkItem:jumper description:@"jumper J"];
    XCTAssertEqual(dispatch_semaphore_wait(jumperRunning, dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC)), 0, @"jumper runs above width");

    [workQueue invalidate];
    XCTAssertEqual([workQueue itemCount], 0);

    XCTestExpectation * nextRan = [self expectationWithDescription:@"item after invalidate ran"];
    MTRAsyncWorkItem * next = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    next.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        completion(MTRAsyncWorkComplete);
        [nextRan fulfill];
    };
    [workQueue enqueueWorkItem:next description:@"next"];
    [self waitForExpectations:@[ nextRan ] timeout:2];
}

- (void)testSendFailureAfterReadyToSendReleasesJumperSlot
{
    // Fifth exit: the send hook ran (item marked sent) and the send itself then failed; the item completes through
    // its normal completion, which releases the jumper slot.
    MTRAsyncWorkQueue * workQueue = [[MTRAsyncWorkQueue alloc] initWithContext:NSNull.null];
    dispatch_semaphore_t readRunning = dispatch_semaphore_create(0);
    __block MTRAsyncWorkCompletionBlock readCompletion;
    MTRAsyncWorkItem * read = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    read.tracksSend = YES;
    read.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        readCompletion = completion;
        dispatch_semaphore_signal(readRunning);
    };
    [workQueue enqueueWorkItem:read description:@"unsent read R"];
    XCTAssertEqual(dispatch_semaphore_wait(readRunning, dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC)), 0);

    NSMutableArray<NSString *> * order = [NSMutableArray array];
    MTRAsyncWorkItem * failing = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    failing.jumpsUnsentWork = YES;
    MTRAsyncWorkItem * __weak weakFailing = failing;
    failing.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        [workQueue workItem:weakFailing readyToSend:^ {}];
        @synchronized(order) {
            [order addObject:@"J1 send failed"];
        }
        completion(MTRAsyncWorkComplete); // the send failed after the hook
    };
    [workQueue enqueueWorkItem:failing description:@"jumper J1"];
    [workQueue enqueueWorkItem:[self _jumperNamed:@"J2" order:order] description:@"jumper J2"];
    [self _waitForOrderCount:2 order:order];
    @synchronized(order) {
        XCTAssertEqualObjects(order, (@[ @"J1 send failed", @"J2" ]), @"the slot is free for the next jumper while R is still unsent");
    }
    readCompletion(MTRAsyncWorkComplete);
}

@end
