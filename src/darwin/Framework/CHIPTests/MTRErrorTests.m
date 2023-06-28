/**
 *    Copyright (c) 2023 Project CHIP Authors
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

#import "MTRTestKeys.h"
#import "MTRTestStorage.h"

#import <os/lock.h>

@interface MTRErrorTests : XCTestCase

@end

@implementation MTRErrorTests

- (void)testErrorSourcePaths
{
    // Capture error logging
    os_unfair_lock lock = OS_UNFAIR_LOCK_INIT;
    os_unfair_lock_t lockPtr = &lock;
    NSMutableSet<NSString *> * errors = [[NSMutableSet alloc] init];
    MTRSetLogCallback(MTRLogTypeError, ^(MTRLogType type, NSString * moduleName, NSString * message) {
        os_unfair_lock_lock(lockPtr);
        [errors addObject:message];
        os_unfair_lock_unlock(lockPtr);
    });

    // Provoke an error in the C++ layer
    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);
    XCTAssertFalse([factory isRunning]);
    __auto_type * storage = [[MTRTestStorage alloc] init];
    __auto_type * factoryParams = [[MTRDeviceControllerFactoryParams alloc] initWithStorage:storage];
    XCTAssertTrue([factory startControllerFactory:factoryParams error:NULL]);
    __auto_type * testKeys = [[MTRTestKeys alloc] init];
    __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithIPK:testKeys.ipk fabricID:@1 nocSigner:testKeys];
    params.vendorID = @0xFFF1u;
    [storage setStorageData:[NSData data] forKey:@"f/1/n"]; // fabric table will refuse to overwrite
    NSError * error;
    XCTAssertNil([factory createControllerOnNewFabric:params error:&error]);
    XCTAssertNotNil(error);
    [factory stopControllerFactory];
    MTRSetLogCallback(MTRLogTypeError, nil);

    // Check the source paths baked into SDK and framework error messages
    NSString * sdkError = [[errors objectsPassingTest:^BOOL(NSString * error, BOOL * stop) {
        return [error containsString:@".cpp:"];
    }] anyObject];

    NSString * sdkSource = [self sourceFileFromErrorString:sdkError];
    XCTAssertTrue([sdkSource hasPrefix:@"src/"], @"sdkSource: %@", sdkSource);

    NSString * frameworkError = [[errors objectsPassingTest:^BOOL(NSString * error, BOOL * stop) {
        return [error containsString:@".mm:"];
    }] anyObject];
    NSString * frameworkSource = [self sourceFileFromErrorString:frameworkError];
    XCTAssertTrue([frameworkSource hasSuffix:@".mm"]);
    XCTAssertFalse([frameworkSource containsString:@"/"], @"frameworkSource: %@", frameworkSource);
}

- (NSString *)sourceFileFromErrorString:(NSString *)error
{
    // "... Error(path/to/source.cpp:123: CHIP Error ...)..."
    // "Creating NSError from path/to/source.cpp:456 CHIP Error ..."
    NSRegularExpression * regex = [NSRegularExpression regularExpressionWithPattern:@"[ (]([^ :]+):[0-9]+: CHIP Error"
                                                                            options:0
                                                                              error:NULL];
    NSTextCheckingResult * result = [regex firstMatchInString:error options:0 range:NSMakeRange(0, error.length)];
    if (!result) {
        return nil;
    }

    return [error substringWithRange:[result rangeAtIndex:1]];
}

- (void)tearDown
{
    [MTRDeviceControllerFactory.sharedInstance stopControllerFactory];
}

@end
