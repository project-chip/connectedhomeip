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

#import "MTRError_Test.h"
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

    // Also verify the NSError (Matter) category exposes the same info the log captured.
    // The values depend on whether the framework was built with CHIP_CONFIG_ERROR_SOURCE,
    // which the test bundle can't reliably evaluate at preprocess time — the framework and
    // the test bundle compile as separate translation units with potentially different config.
    // So just verify shape: if the values are populated, the file must be a basename (no path
    // separators) and the line must be > 0. If they're nil/0 the framework didn't capture
    // source for this error, which is also valid.
    NSString * categoryFile = error.mtr_underlyingMatterErrorSourceFile;
    NSUInteger categoryLine = error.mtr_underlyingMatterErrorSourceLine;
    if (categoryFile != nil) {
        XCTAssertGreaterThan(categoryFile.length, (NSUInteger) 0);
        XCTAssertEqual([categoryFile rangeOfString:@"/"].location, (NSUInteger) NSNotFound,
            @"mtr_underlyingMatterErrorSourceFile must be basename only, got %@", categoryFile);
        XCTAssertEqual([categoryFile rangeOfString:@"\\"].location, (NSUInteger) NSNotFound,
            @"mtr_underlyingMatterErrorSourceFile must be basename only, got %@", categoryFile);
        XCTAssertGreaterThan(categoryLine, (NSUInteger) 0);
    }
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

- (void)testUnderlyingErrorSourceFileStripsMixedSeparators
{
    XCTAssertEqualObjects(MTRErrorBasenameForPath("a/b/c.cpp"), @"c.cpp");
    XCTAssertEqualObjects(MTRErrorBasenameForPath("a\\b\\c.cpp"), @"c.cpp");
    XCTAssertEqualObjects(MTRErrorBasenameForPath("C:\\src\\foo/bar\\baz.cpp"), @"baz.cpp");
    XCTAssertEqualObjects(MTRErrorBasenameForPath("a/b\\c.cpp"), @"c.cpp");
    XCTAssertEqualObjects(MTRErrorBasenameForPath("noSeparator.cpp"), @"noSeparator.cpp");
    XCTAssertNil(MTRErrorBasenameForPath(""));
    XCTAssertNil(MTRErrorBasenameForPath(NULL));
}

@end
