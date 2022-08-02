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
        NSLog(@"Item 1 called with counter %d", counter);
        sleep(1);
        counter++;
        NSLog(@"Item 1 woke after sleep with counter %d", counter);
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
        NSLog(@"Item 2 called with counter %d", counter);
        if (counter == 1) {
            [expectation fulfill];
        }
        [workItem2 endWork];
    };
    workItem2.readyHandler = readyHandler2;
    workItem2.cancelHandler = ^{
    };
    [workQueue enqueueWorkItem:workItem2];

    NSLog(@"2Items start wait %@", [NSDate date]);
    [self waitForExpectationsWithTimeout:5 handler:nil];
    NSLog(@"2Items finished wait %@", [NSDate date]);

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
        NSLog(@"Item 1 called with counter %d retryCount %lu", counter, (unsigned long) retryCount);
        sleep(1);
        counter++;
        NSLog(@"Item 1 woke after sleep with counter %d", counter);

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
        NSLog(@"Item 2 called with counter %d", counter);
        if (counter == 2) {
            [expectation fulfill];
        }
        [workItem2 endWork];
    };
    workItem2.readyHandler = readyHandler2;
    workItem2.cancelHandler = ^{
    };
    [workQueue enqueueWorkItem:workItem2];

    NSLog(@"2Items start wait %@", [NSDate date]);
    [self waitForExpectationsWithTimeout:5 handler:nil];
    NSLog(@"2Items finished wait %@", [NSDate date]);

    // see that workItem1 ran twice after the retry
    XCTAssertEqual(counter, 2);
}

@end
