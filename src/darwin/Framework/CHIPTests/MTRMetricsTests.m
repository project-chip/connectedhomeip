/*
 *
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
#import <Matter/Matter.h>

#import "MTRMetrics_Internal.h"

// system dependencies
#import <XCTest/XCTest.h>

@interface MTRMetricsTests : XCTestCase
@end

@implementation MTRMetricsTests

- (void)setUp
{
    // Per-test setup, runs before each test.
    [super setUp];
}

- (void)tearDown
{
    // Per-test teardown, runs after each test.
    [super tearDown];
}

- (void)test000_SetUp
{
    // Nothing to do here; our setUp method handled this already.  This test
    // just exists to make the setup not look like it's happening inside other
    // tests.
}

- (void)test001_TestAllKeys
{
    MTRMetrics * metrics = [[MTRMetrics alloc] initWithCapacity:4];
    [metrics setValue:@"metricsCounter1" forKey:@"com.matter.metrics.counter1"];
    [metrics setValue:@"metricsCounter2" forKey:@"com.matter.metrics.counter2"];
    [metrics setValue:@"metricsCounter3" forKey:@"com.matter.metrics.counter3"];
    [metrics setValue:@"metricsCounter4" forKey:@"com.matter.metrics.counter4"];

    NSArray<NSString *> * keys = [metrics allKeys];
    XCTAssertTrue([keys count] == 4);
    XCTAssertTrue([keys containsObject:@"com.matter.metrics.counter1"]);
    XCTAssertTrue([keys containsObject:@"com.matter.metrics.counter2"]);
    XCTAssertTrue([keys containsObject:@"com.matter.metrics.counter3"]);
    XCTAssertTrue([keys containsObject:@"com.matter.metrics.counter4"]);
    XCTAssertFalse([keys containsObject:@"com.matter.metrics.counter5"]);
}
- (void)test002_TestOneKey
{
    MTRMetrics * metrics = [[MTRMetrics alloc] initWithCapacity:1];
    [metrics setValue:@"metricsCounter1" forKey:@"com.matter.metrics.counter1"];

    NSArray<NSString *> * keys = [metrics allKeys];
    XCTAssertTrue([keys count] == 1);
    XCTAssertTrue([keys containsObject:@"com.matter.metrics.counter1"]);
}

- (void)test003_TestMultipleKeys
{
    MTRMetrics * metrics = [[MTRMetrics alloc] initWithCapacity:3];
    [metrics setValue:@"metricsCounter1" forKey:@"com.matter.metrics.counter1"];
    [metrics setValue:@"metricsCounter2" forKey:@"com.matter.metrics.counter2"];
    [metrics setValue:[NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeInvalidState userInfo:nil] forKey:@"com.matter.metrics.counter3"];

    NSArray<NSString *> * keys = [metrics allKeys];
    XCTAssertTrue([keys count] == 3);
    XCTAssertTrue([keys containsObject:@"com.matter.metrics.counter1"]);
    XCTAssertTrue([keys containsObject:@"com.matter.metrics.counter2"]);
    XCTAssertTrue([keys containsObject:@"com.matter.metrics.counter3"]);
    XCTAssertFalse([keys containsObject:@"com.matter.metrics.counter4"]);
}

- (void)test004_TestValueForKey
{
    MTRMetrics * metrics = [[MTRMetrics alloc] initWithCapacity:1];
    [metrics setValue:@"metricsCounter1" forKey:@"com.matter.metrics.counter1"];

    XCTAssertEqualObjects([metrics valueForKey:@"com.matter.metrics.counter1"], @"metricsCounter1");
}

- (void)test005_TestMultipleValueForKeys
{
    MTRMetrics * metrics = [[MTRMetrics alloc] initWithCapacity:3];
    [metrics setValue:@"metricsCounter1" forKey:@"com.matter.metrics.counter1"];
    [metrics setValue:@"metricsCounter2" forKey:@"com.matter.metrics.counter2"];
    [metrics setValue:[NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeInvalidState userInfo:nil] forKey:@"com.matter.metrics.counter3"];

    XCTAssertEqualObjects([metrics valueForKey:@"com.matter.metrics.counter1"], @"metricsCounter1");
    XCTAssertEqualObjects([metrics valueForKey:@"com.matter.metrics.counter2"], @"metricsCounter2");
    XCTAssertEqualObjects([metrics valueForKey:@"com.matter.metrics.counter3"], [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeInvalidState userInfo:nil]);
}

- (void)test006_TestValueRemoval
{
    MTRMetrics * metrics = [[MTRMetrics alloc] initWithCapacity:2];
    [metrics setValue:@"metricsCounter1" forKey:@"com.matter.metrics.counter1"];
    [metrics setValue:@"metricsCounter2" forKey:@"com.matter.metrics.counter2"];

    NSArray<NSString *> * keys = [metrics allKeys];
    XCTAssertTrue([keys count] == 2);

    [metrics setValue:nil forKey:@"com.matter.metrics.counter2"];
    keys = [metrics allKeys];
    XCTAssertTrue([keys count] == 1);
    XCTAssertTrue([keys containsObject:@"com.matter.metrics.counter1"]);
    XCTAssertFalse([keys containsObject:@"com.matter.metrics.counter2"]);

    [metrics setValue:nil forKey:@"com.matter.metrics.counter1"];
    keys = [metrics allKeys];
    XCTAssertTrue([keys count] == 0);
}

@end
