/*
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

// module headers
#import <Matter/Matter.h>

#import "MTRTestCase+ServerAppRunner.h"
#import "MTRTestCase.h"
#import "MTRTestDeclarations.h"

static const uint64_t kDeviceId = 0x12344321;
static MTRDeviceController * sDeviceController = nil;
static const uint16_t kTimeoutInSeconds = 10;
static NSString * shortLogContent = @"This is a short log\n";
static NSString * longLogContent = nil;

@interface MTRDiagnosticLogDownloadTests : MTRTestCase

@end

@implementation MTRDiagnosticLogDownloadTests

+ (void)setUp
{
    [super setUp];

    // TODO: Once we have a way to startCommissionedAppWithName with explicit
    // per-suite scope, do that in the instance setUp method, with a "have we
    // already done it?" guard.

    // Ensure that our long log content is long enough that it can't fit in a
    // 1280-byte packet, so has to be sent via BDX.  In particular, ensure we
    // have at least 2KB of log.
    NSUInteger neededSize = 2048;

    NSUInteger expectedCopies = (neededSize - 1) / shortLogContent.length + 1;
    NSUInteger expectedSize = expectedCopies * shortLogContent.length;
    NSMutableString * mutableLongLogContent = [NSMutableString stringWithCapacity:expectedSize];
    while (mutableLongLogContent.length < neededSize) {
        [mutableLongLogContent appendString:shortLogContent];
    }
    longLogContent = mutableLongLogContent;

    NSString * endUserSupportLog = [self _createLogFile:shortLogContent];
    NSString * networkDiagnosticsLog = [self _createLogFile:longLogContent];
    NSString * crashLog = [self _createLogFile:longLogContent];

    sDeviceController = [self startCommissionedAppWithName:@"all-clusters"
                                                 arguments:@[
                                                     @("--end_user_support_log"),
                                                     endUserSupportLog,
                                                     @("--network_diagnostics_log"),
                                                     networkDiagnosticsLog,
                                                     @("--crash_log"),
                                                     crashLog
                                                 ]
                                                    nodeID:@(kDeviceId)];
    XCTAssertNotNil(sDeviceController);
}

+ (NSString *)_createLogFile:(NSString *)logContent
{
    NSString * uniqueName = [[NSUUID UUID] UUIDString];
    NSString * logFilePath = [NSTemporaryDirectory() stringByAppendingPathComponent:uniqueName];
    BOOL created = [[NSFileManager defaultManager] createFileAtPath:logFilePath
                                                           contents:[logContent dataUsingEncoding:NSUTF8StringEncoding]
                                                         attributes:nil];
    XCTAssertTrue(created);
    return logFilePath;
}

- (void)setUp
{
    [super setUp];
    [self setContinueAfterFailure:NO];
}

- (void)_testDownloadLogWithContent:(NSString *)expectedLogContent type:(MTRDiagnosticLogType)type testName:(NSString *)testName
{

    XCTestExpectation * expectation =
        [self expectationWithDescription:@"Downloaded the end user support log"];

    MTRDevice * device = [MTRDevice deviceWithNodeID:@(kDeviceId) controller:sDeviceController];
    XCTAssertNotNil(device, "%@", testName);

    dispatch_queue_t queue = dispatch_get_main_queue();

    [device downloadLogOfType:type
                      timeout:kTimeoutInSeconds
                        queue:queue
                   completion:^(NSURL * _Nullable url, NSError * _Nullable error) {
                       NSLog(@"downloadLogOfType: url: %@, error: %@", url, error);
                       XCTAssertNil(error, "%@", testName);

                       XCTAssertNotNil(url, "%@", testName);

                       NSError * readError;
                       NSString * fileContent = [NSString stringWithContentsOfURL:url encoding:NSUTF8StringEncoding error:&readError];
                       XCTAssertNil(readError, "%@", testName);
                       XCTAssertEqualObjects(fileContent, expectedLogContent, "%@", testName);
                       [expectation fulfill];
                   }];

    [self waitForExpectations:@[ expectation ] timeout:kTimeoutInSeconds];

    // TODO: BDXDiagnosticLogsProvider.cpp has a 50ms lag between receiving the
    // BlockAckEOF message and actually treating the transfer as done, because
    // it does the 50ms poll thing.  Wait 100ms to make sure it has time to
    // process the message.
    usleep(100 * 1000);
}

- (void)test001_UserSupportLog
{
    [self _testDownloadLogWithContent:shortLogContent type:MTRDiagnosticLogTypeEndUserSupport testName:@("test001_UserSupportLog")];
}

- (void)test002_NetworkDiagnosticsLog
{
    [self _testDownloadLogWithContent:longLogContent type:MTRDiagnosticLogTypeNetworkDiagnostics testName:@("test002_NetworkDiagnosticsLog")];
}

- (void)test003_CrashLog
{
    [self _testDownloadLogWithContent:longLogContent type:MTRDiagnosticLogTypeCrash testName:@("test003_CrashLog")];
}

- (void)test004_CanceledDownload
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"Canceled download completed"];

    MTRDevice * device = [MTRDevice deviceWithNodeID:@(kDeviceId) controller:sDeviceController];
    XCTAssertNotNil(device);

    dispatch_queue_t queue = dispatch_get_main_queue();

    [device downloadLogOfType:MTRDiagnosticLogTypeEndUserSupport
                      timeout:kTimeoutInSeconds
                        queue:queue
                   completion:^(NSURL * _Nullable url, NSError * _Nullable error) {
                       XCTAssertNil(url);
                       XCTAssertNotNil(error);

                       XCTAssertEqual(error.domain, MTRErrorDomain);
                       XCTAssertEqual(error.code, MTRErrorCodeCancelled);
                       [expectation fulfill];
                   }];

    [self _testDownloadLogWithContent:shortLogContent type:MTRDiagnosticLogTypeEndUserSupport testName:@("test004_CanceledDownload")];

    [self waitForExpectations:@[ expectation ] timeout:kTimeoutInSeconds];
}

@end
