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
#import <objc/runtime.h>

#include <fcntl.h>
#include <stdlib.h>
#include <sys/param.h>
#include <unistd.h>

#import "MTRTestCase+ServerAppRunner.h"
#import "MTRTestCase.h"
#import "MTRTestDeclarations.h"

static const uint64_t kDeviceId = 0x12344321;
static MTRDeviceController * sDeviceController = nil;
static const uint16_t kTimeoutInSeconds = 10;
static NSString * shortLogContent = @"This is a short log\n";
static NSString * longLogContent = nil;
static NSString * crashLogContent = nil;

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

    NSMutableString * mutableCrashLogContent = [NSMutableString string];
    while (mutableCrashLogContent.length < 16 * 1024) {
        [mutableCrashLogContent appendString:shortLogContent];
    }
    crashLogContent = mutableCrashLogContent;

    NSString * endUserSupportLog = [self _createLogFile:shortLogContent];
    NSString * networkDiagnosticsLog = [self _createLogFile:longLogContent];
    NSString * crashLog = [self _createLogFile:crashLogContent];

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

- (void)_checkPartialCrashLogAtURL:(NSURL * _Nullable)url
{
    XCTAssertNotNil(url);
    NSString * partialLog = [NSString stringWithContentsOfURL:url encoding:NSUTF8StringEncoding error:nil];
    XCTAssertGreaterThan(partialLog.length, 0);
    XCTAssertLessThan(partialLog.length, crashLogContent.length);
    XCTAssertTrue(partialLog != nil && [crashLogContent hasPrefix:partialLog]);
}

- (XCTestExpectation *)_startCrashDownloadAndWaitForTransfer
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"Download canceled mid-transfer completed"];

    MTRDevice * device = [MTRDevice deviceWithNodeID:@(kDeviceId) controller:sDeviceController];
    XCTAssertNotNil(device);

    NSString * tempDir = NSTemporaryDirectory();
    NSFileManager * fileManager = [NSFileManager defaultManager];
    NSSet * existingFiles = [NSSet setWithArray:[fileManager contentsOfDirectoryAtPath:tempDir error:nil]];
    NSString * crashLogSuffix = [NSString stringWithFormat:@"_%016llX_Crash", kDeviceId];

    [device downloadLogOfType:MTRDiagnosticLogTypeCrash
                      timeout:kTimeoutInSeconds
                        queue:dispatch_get_main_queue()
                   completion:^(NSURL * _Nullable url, NSError * _Nullable error) {
                       XCTAssertEqualObjects(error.domain, MTRErrorDomain);
                       XCTAssertEqual(error.code, MTRErrorCodeCancelled);
                       [self _checkPartialCrashLogAtURL:url];
                       [expectation fulfill];
                   }];

    BOOL transferStarted = NO;
    NSDate * deadline = [NSDate dateWithTimeIntervalSinceNow:kTimeoutInSeconds];
    while (!transferStarted && [deadline timeIntervalSinceNow] > 0) {
        [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.01]];
        for (NSString * file in [fileManager contentsOfDirectoryAtPath:tempDir error:nil]) {
            if ([file hasSuffix:crashLogSuffix] && ![existingFiles containsObject:file]) {
                transferStarted = YES;
                break;
            }
        }
    }
    XCTAssertTrue(transferStarted);
    return expectation;
}

- (void)_testDownloadAfterCanceledTransferWithContent:(NSString *)expectedLogContent type:(MTRDiagnosticLogType)type testName:(NSString *)testName
{
    XCTestExpectation * expectation = [self _startCrashDownloadAndWaitForTransfer];

    [self _testDownloadLogWithContent:expectedLogContent type:type testName:testName];

    [self waitForExpectations:@[ expectation ] timeout:kTimeoutInSeconds];
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
    [self _testDownloadLogWithContent:crashLogContent type:MTRDiagnosticLogTypeCrash testName:@("test003_CrashLog")];
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

- (void)test005_CanceledDownloadAfterTransferStarted
{
    [self _testDownloadAfterCanceledTransferWithContent:shortLogContent type:MTRDiagnosticLogTypeEndUserSupport testName:@("test005_CanceledDownloadAfterTransferStarted")];
}

- (BOOL)_isFileOpenAtURL:(NSURL *)url
{
    char expected[MAXPATHLEN];
    char path[MAXPATHLEN];
    if (realpath(url.fileSystemRepresentation, expected) == NULL) {
        return NO;
    }
    for (int fd = 0; fd < getdtablesize(); fd++) {
        if (fcntl(fd, F_GETPATH, path) == 0 && strcmp(path, expected) == 0) {
            return YES;
        }
    }
    return NO;
}

- (void)test006_TimedOutDownloadAfterTransferStarted
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"Download timed out mid-transfer completed"];

    MTRDevice * device = [MTRDevice deviceWithNodeID:@(kDeviceId) controller:sDeviceController];
    XCTAssertNotNil(device);

    __block NSURL * timedOutURL;
    __block NSError * timedOutError;
    [device downloadLogOfType:MTRDiagnosticLogTypeCrash
                      timeout:2
                        queue:dispatch_get_main_queue()
                   completion:^(NSURL * _Nullable url, NSError * _Nullable error) {
                       timedOutURL = url;
                       timedOutError = error;
                       [expectation fulfill];
                   }];

    [self waitForExpectations:@[ expectation ] timeout:kTimeoutInSeconds];
    XCTAssertEqualObjects(timedOutError.domain, MTRErrorDomain);
    XCTAssertEqual(timedOutError.code, MTRErrorCodeTimeout);
    [self _checkPartialCrashLogAtURL:timedOutURL];
    if (nil == timedOutURL) {
        return;
    }

    NSPredicate * fileClosed = [NSPredicate predicateWithBlock:^BOOL(id object, NSDictionary * bindings) {
        return ![self _isFileOpenAtURL:timedOutURL];
    }];
    XCTNSPredicateExpectation * closed = [[XCTNSPredicateExpectation alloc] initWithPredicate:fileClosed object:nil];
    [self waitForExpectations:@[ closed ] timeout:kTimeoutInSeconds];
}

- (void)test007_DownloadAfterCanceledTransfer
{
    [self _testDownloadAfterCanceledTransferWithContent:longLogContent type:MTRDiagnosticLogTypeNetworkDiagnostics testName:@("test007_DownloadAfterCanceledTransfer")];
}

- (void)test008_DownloadAfterTimedOutTransfer
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"Download timed out mid-transfer completed"];

    MTRDevice * device = [MTRDevice deviceWithNodeID:@(kDeviceId) controller:sDeviceController];
    XCTAssertNotNil(device);

    [device downloadLogOfType:MTRDiagnosticLogTypeCrash
                      timeout:2
                        queue:dispatch_get_main_queue()
                   completion:^(NSURL * _Nullable url, NSError * _Nullable error) {
                       XCTAssertEqualObjects(error.domain, MTRErrorDomain);
                       XCTAssertEqual(error.code, MTRErrorCodeTimeout);
                       [self _checkPartialCrashLogAtURL:url];
                       [expectation fulfill];
                   }];

    [self waitForExpectations:@[ expectation ] timeout:kTimeoutInSeconds];

    [self _testDownloadLogWithContent:longLogContent type:MTRDiagnosticLogTypeNetworkDiagnostics testName:@("test008_DownloadAfterTimedOutTransfer")];
}

- (void)test009_DownloadWhenTimedOutTransferEndsBeforeItsCompletion
{
    XCTestExpectation * timedOutExpectation = [self expectationWithDescription:@"Download timed out mid-transfer completed"];
    XCTestExpectation * nextExpectation = [self expectationWithDescription:@"Next download completed"];

    MTRDevice * device = [MTRDevice deviceWithNodeID:@(kDeviceId) controller:sDeviceController];
    XCTAssertNotNil(device);

    SEL sessionEnd = NSSelectorFromString(@"handleBDXTransferSessionEndForFileDesignator:fabricIndex:nodeID:error:");
    Method method = class_getInstanceMethod(NSClassFromString(@"MTRDiagnosticLogsDownloader"), sessionEnd);
    XCTAssertTrue(method != NULL);
    if (method == NULL) {
        return;
    }
    IMP originalImp = method_getImplementation(method);

    // Holding the timed-out download's completion also holds its removal, so the transfer session ends first.
    dispatch_queue_t timedOutQueue = dispatch_queue_create("MTRDiagnosticLogDownloadTests.timedOut", DISPATCH_QUEUE_SERIAL);
    dispatch_suspend(timedOutQueue);
    __block BOOL timedOutQueueSuspended = YES;

    __block NSURL * nextURL = nil;
    __block NSError * nextError = nil;
    __block BOOL requestedNext = NO;
    IMP newImp = imp_implementationWithBlock(^(id downloader, NSString * fileDesignator, NSNumber * fabricIndex, NSNumber * nodeID, NSError * _Nullable error) {
        ((void (*)(id, SEL, NSString *, NSNumber *, NSNumber *, NSError *)) originalImp)(downloader, sessionEnd, fileDesignator, fabricIndex, nodeID, error);
        if (requestedNext || ![fileDesignator hasSuffix:@"/Crash"]) {
            return;
        }
        requestedNext = YES;
        [device downloadLogOfType:MTRDiagnosticLogTypeNetworkDiagnostics
                          timeout:kTimeoutInSeconds
                            queue:dispatch_get_main_queue()
                       completion:^(NSURL * _Nullable url, NSError * _Nullable nextDownloadError) {
                           nextURL = url;
                           nextError = nextDownloadError;
                           [nextExpectation fulfill];
                       }];
    });
    method_setImplementation(method, newImp);
    [self addTeardownBlock:^{
        method_setImplementation(method, originalImp);
        imp_removeBlock(newImp);
        if (timedOutQueueSuspended) {
            dispatch_resume(timedOutQueue);
        }
    }];

    __block NSURL * timedOutURL = nil;
    __block NSError * timedOutError = nil;
    [device downloadLogOfType:MTRDiagnosticLogTypeCrash
                      timeout:2
                        queue:timedOutQueue
                   completion:^(NSURL * _Nullable url, NSError * _Nullable error) {
                       timedOutURL = url;
                       timedOutError = error;
                       [timedOutExpectation fulfill];
                   }];

    [self waitForExpectations:@[ nextExpectation ] timeout:kTimeoutInSeconds];
    timedOutQueueSuspended = NO;
    dispatch_resume(timedOutQueue);
    [self waitForExpectations:@[ timedOutExpectation ] timeout:kTimeoutInSeconds];

    XCTAssertEqualObjects(timedOutError.domain, MTRErrorDomain);
    XCTAssertEqual(timedOutError.code, MTRErrorCodeTimeout);
    [self _checkPartialCrashLogAtURL:timedOutURL];

    XCTAssertNil(nextError);
    XCTAssertEqualObjects([NSString stringWithContentsOfURL:nextURL encoding:NSUTF8StringEncoding error:nil], longLogContent);
    usleep(100 * 1000);
}

- (void)test010_DownloadStopsRetryingAfterMaxBusyResponses
{
    const NSUInteger kMaxBusyRetries = 3;

    MTRDevice * device = [MTRDevice deviceWithNodeID:@(kDeviceId) controller:sDeviceController];
    XCTAssertNotNil(device);

    SEL retrieveLogs = @selector(retrieveLogsRequestWithParams:expectedValues:expectedValueInterval:completion:);
    Method sendMethod = class_getInstanceMethod([MTRClusterDiagnosticLogs class], retrieveLogs);
    SEL retry = NSSelectorFromString(@"retryRequestAfterBusy");
    Method retryMethod = class_getInstanceMethod(NSClassFromString(@"MTRDownload"), retry);
    XCTAssertTrue(sendMethod != NULL && retryMethod != NULL);
    if (sendMethod == NULL || retryMethod == NULL) {
        return;
    }
    IMP originalSend = method_getImplementation(sendMethod);
    IMP originalRetry = method_getImplementation(retryMethod);

    __block NSUInteger sends = 0;
    NSMutableArray<NSNumber *> * retries = [NSMutableArray array];
    IMP busySend = imp_implementationWithBlock(^(id cluster, MTRDiagnosticLogsClusterRetrieveLogsRequestParams * params, NSArray * _Nullable expectedValues,
        NSNumber * _Nullable expectedValueInterval, void (^completion)(MTRDiagnosticLogsClusterRetrieveLogsResponseParams * _Nullable, NSError * _Nullable)) {
        if (![params.intent isEqual:@(MTRDiagnosticLogTypeNetworkDiagnostics)]) {
            ((void (*)(id, SEL, id, id, id, id)) originalSend)(cluster, retrieveLogs, params, expectedValues, expectedValueInterval, completion);
            return;
        }
        sends++;
        MTRDiagnosticLogsClusterRetrieveLogsResponseParams * response = [[MTRDiagnosticLogsClusterRetrieveLogsResponseParams alloc] init];
        response.status = @(MTRDiagnosticLogsStatusBusy);
        dispatch_async(dispatch_get_main_queue(), ^{
            completion(response, nil);
        });
    });
    IMP recordingRetry = imp_implementationWithBlock(^BOOL(id download) {
        BOOL retried = ((BOOL(*)(id, SEL)) originalRetry)(download, retry);
        [retries addObject:@(retried)];
        return retried;
    });
    method_setImplementation(sendMethod, busySend);
    method_setImplementation(retryMethod, recordingRetry);
    [self addTeardownBlock:^{
        method_setImplementation(sendMethod, originalSend);
        method_setImplementation(retryMethod, originalRetry);
        imp_removeBlock(busySend);
        imp_removeBlock(recordingRetry);
    }];

    XCTestExpectation * canceledExpectation = [self _startCrashDownloadAndWaitForTransfer];

    XCTestExpectation * busyExpectation = [self expectationWithDescription:@"Download answered Busy completed"];
    __block NSURL * busyURL = nil;
    __block NSError * busyError = nil;
    [device downloadLogOfType:MTRDiagnosticLogTypeNetworkDiagnostics
                      timeout:kTimeoutInSeconds
                        queue:dispatch_get_main_queue()
                   completion:^(NSURL * _Nullable url, NSError * _Nullable error) {
                       busyURL = url;
                       busyError = error;
                       [busyExpectation fulfill];
                   }];

    [self waitForExpectations:@[ canceledExpectation, busyExpectation ] timeout:kTimeoutInSeconds];

    XCTAssertNil(busyURL);
    XCTAssertEqualObjects(busyError.domain, MTRErrorDomain);
    XCTAssertEqual(busyError.code, MTRErrorCodeBusy);
    XCTAssertEqual(sends, kMaxBusyRetries + 1);
    NSArray<NSNumber *> * expectedRetries = @[ @YES, @YES, @YES, @NO ];
    XCTAssertEqualObjects(retries, expectedRetries);
}

@end
