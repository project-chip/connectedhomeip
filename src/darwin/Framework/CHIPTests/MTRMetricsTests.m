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
#import "MTRSecureCodingTestHelpers.h"

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
    MTRMetricData * metricData1 = [MTRMetricData new];
    MTRMetricData * metricData2 = [MTRMetricData new];
    MTRMetricData * metricData3 = [MTRMetricData new];
    MTRMetricData * metricData4 = [MTRMetricData new];
    [metrics setMetricData:metricData1 forKey:@"com.matter.metrics.counter1"];
    [metrics setMetricData:metricData2 forKey:@"com.matter.metrics.counter2"];
    [metrics setMetricData:metricData3 forKey:@"com.matter.metrics.counter3"];
    [metrics setMetricData:metricData4 forKey:@"com.matter.metrics.counter4"];

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
    MTRMetricData * metricData1 = [MTRMetricData new];
    [metrics setMetricData:metricData1 forKey:@"com.matter.metrics.counter1"];

    NSArray<NSString *> * keys = [metrics allKeys];
    XCTAssertTrue([keys count] == 1);
    XCTAssertTrue([keys containsObject:@"com.matter.metrics.counter1"]);
}

- (void)test003_TestValueForKey
{
    MTRMetrics * metrics = [[MTRMetrics alloc] initWithCapacity:1];
    MTRMetricData * metricData1 = [MTRMetricData new];
    [metrics setMetricData:metricData1 forKey:@"com.matter.metrics.counter1"];

    XCTAssertEqualObjects([metrics metricDataForKey:@"com.matter.metrics.counter1"], metricData1);
}

- (void)test004_TestMultipleValueForKeys
{
    MTRMetrics * metrics = [[MTRMetrics alloc] initWithCapacity:3];
    MTRMetricData * metricData1 = [MTRMetricData new];
    MTRMetricData * metricData2 = [MTRMetricData new];
    MTRMetricData * metricData3 = [MTRMetricData new];
    [metrics setMetricData:metricData1 forKey:@"com.matter.metrics.counter1"];
    [metrics setMetricData:metricData2 forKey:@"com.matter.metrics.counter2"];
    [metrics setMetricData:metricData3 forKey:@"com.matter.metrics.counter3"];

    XCTAssertEqualObjects([metrics metricDataForKey:@"com.matter.metrics.counter1"], metricData1);
    XCTAssertEqualObjects([metrics metricDataForKey:@"com.matter.metrics.counter2"], metricData2);
    XCTAssertEqualObjects([metrics metricDataForKey:@"com.matter.metrics.counter3"], metricData3);
    XCTAssertNotEqualObjects([metrics metricDataForKey:@"com.matter.metrics.counter3"], metricData2);
}

- (void)test005_TestValueRemoval
{
    MTRMetrics * metrics = [[MTRMetrics alloc] initWithCapacity:2];
    MTRMetricData * metricData1 = [MTRMetricData new];
    MTRMetricData * metricData2 = [MTRMetricData new];
    [metrics setMetricData:metricData1 forKey:@"com.matter.metrics.counter1"];
    [metrics setMetricData:metricData2 forKey:@"com.matter.metrics.counter2"];

    NSArray<NSString *> * keys = [metrics allKeys];
    XCTAssertTrue([keys count] == 2);

    [metrics setMetricData:nil forKey:@"com.matter.metrics.counter2"];
    keys = [metrics allKeys];
    XCTAssertTrue([keys count] == 1);
    XCTAssertTrue([keys containsObject:@"com.matter.metrics.counter1"]);
    XCTAssertFalse([keys containsObject:@"com.matter.metrics.counter2"]);
    XCTAssertEqualObjects([metrics metricDataForKey:@"com.matter.metrics.counter1"], metricData1);
    XCTAssertNotEqualObjects([metrics metricDataForKey:@"com.matter.metrics.counter1"], metricData2);

    [metrics setMetricData:nil forKey:@"com.matter.metrics.counter1"];
    keys = [metrics allKeys];
    XCTAssertTrue([keys count] == 0);
}

- (void)test006_TestUniqueIdentifier
{
    MTRMetrics * metrics1 = [[MTRMetrics alloc] initWithCapacity:1];
    MTRMetrics * metrics2 = [[MTRMetrics alloc] initWithCapacity:1];
    XCTAssertNotNil(metrics1.uniqueIdentifier);
    XCTAssertNotNil(metrics2.uniqueIdentifier);
    XCTAssertNotEqualObjects(metrics1.uniqueIdentifier, metrics2.uniqueIdentifier);
}

- (void)test007_TestSecureCodingRoundTrip
{
    MTRMetricData * data = [MTRMetricData new];
    [data setValue:@(5) forKey:@"value"];
    [data setValue:@(0x32) forKey:@"errorCode"];
    [data setValue:@(1.5) forKey:@"duration"];
    [data setValue:@(1) forKey:@"type"];
    MTRMetrics * metrics = [[MTRMetrics alloc] initWithCapacity:1];
    [metrics setMetricData:data forKey:@"com.matter.metrics.counter1"];

    NSError * error;
    MTRMetrics * decoded = RoundTripEncodable((id<NSSecureCoding>) metrics, &error);
    XCTAssertNil(error);
    XCTAssertEqualObjects(decoded.uniqueIdentifier, metrics.uniqueIdentifier);
    XCTAssertEqualObjects(decoded.allKeys, metrics.allKeys);
    MTRMetricData * decodedData = [decoded metricDataForKey:@"com.matter.metrics.counter1"];
    XCTAssertEqualObjects(decodedData.value, @(5));
    XCTAssertEqualObjects(decodedData.errorCode, @(0x32));
    XCTAssertEqualObjects(decodedData.duration, @(1.5));
    XCTAssertEqualObjects(decodedData.description, data.description);

    if (![MTRMetrics instancesRespondToSelector:@selector(initWithCoder:)] || ![MTRMetricData instancesRespondToSelector:@selector(initWithCoder:)]) {
        XCTFail("MTRMetrics or MTRMetricData does not adopt NSCoding");
        return;
    }
    XCTAssertNil([self _decode:MTRMetrics.class objects:@{ @"id" : metrics.uniqueIdentifier } type:nil]);
    XCTAssertNil([self _decode:MTRMetrics.class objects:@{ @"md" : @ {} } type:nil]);
    NSString * instant = [MTRMetricData new].description;
    XCTAssertEqualObjects([[self _decode:MTRMetricData.class objects:@{} type:nil] description], instant);
    XCTAssertEqualObjects([[self _decode:MTRMetricData.class objects:@{} type:@(99)] description], instant);
    XCTAssertEqualObjects([[self _decode:MTRMetricData.class objects:@{} type:@(-1)] description], instant);
}

- (id)_decode:(Class)cls objects:(NSDictionary<NSString *, id> *)objects type:(NSNumber * _Nullable)type
{
    NSKeyedArchiver * archiver = [[NSKeyedArchiver alloc] initRequiringSecureCoding:YES];
    [objects enumerateKeysAndObjectsUsingBlock:^(NSString * key, id value, BOOL * stop) {
        [archiver encodeObject:value forKey:key];
    }];
    if (type != nil) {
        [archiver encodeInteger:type.integerValue forKey:@"t"];
    }
    [archiver finishEncoding];
    NSError * error;
    NSKeyedUnarchiver * unarchiver = [[NSKeyedUnarchiver alloc] initForReadingFromData:archiver.encodedData error:&error];
    XCTAssertNil(error);
    return [(id<NSCoding>) [cls alloc] initWithCoder:unarchiver];
}

@end
